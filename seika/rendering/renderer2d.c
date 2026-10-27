#if SKA_RENDERING

// Must precede renderer.h: windows.h (pulled in here) redefines APIENTRY before glad.h can guard it (C4005).
#include "seika/window/window.h"

#include "renderer.h"

#include <stdlib.h>
#include <string.h>

#include "render_context.h"
#include "shader/shader.h"
#include "shader/shader_cache.h"
#include "shader/shader_source.h"
#include "seika/assert.h"
#include "../logger.h"
#include "seika/data_structures/static_array.h"

#define SKA_RENDER_LAYER_BATCH_ITEM_MAX 1024
#define SKA_RENDER_LAYER_FONT_BATCH_ITEM_MAX 1024
#define SKA_RENDER_TEXTURE_LAYER_TEXTURE_MAX 64

typedef struct SkaTextureCoordinates {
    GLfloat s_min;
    GLfloat s_max;
    GLfloat t_min;
    GLfloat t_max;
} SkaTextureCoordinates;

static SkaTextureCoordinates renderer_get_texture_coordinates(const SkaTexture* texture, const SkaRect2* draw_source, bool flip_h, bool flip_v);

static void sprite_renderer_initialize();
static void sprite_renderer_finalize();
static void sprite_renderer_update_resolution();

static void font_renderer_initialize();
static void font_renderer_finalize();
static void font_renderer_update_resolution();
static void font_renderer_draw_text(const SkaFont* font, const char* text, f32 x, f32 y, f32 scale, const SkaColor* color);

static GLuint sprite_quad_vao;
static GLuint sprite_quad_vbo;

// 2D
static SkaShader* sprite_shader = NULL;
static SkaShader* font_shader = NULL;

// Global Shader Params
static f32 global_shader_param_time = 0.0f;

static f32 resolution_width = 800.0f;
static f32 resolution_height = 600.0f;
static SkaMatrix4 sprite_projection = {0};

// Draw Batching
typedef struct SpriteBatchItem {
    SkaTexture* texture;
    SkaRect2 source_rect;
    SkaSize2D dest_size;
    SkaColor color;
    bool flip_h;
    bool flip_v;
    SkaRendererTransform2D transform2_d;
    SkaShaderInstance* shader_instance;
} SpriteBatchItem;

// Font draw calls are deferred to the end-of-frame flush (ska_renderer_flush_2d_batches), so 'text'
// must be a real owned copy, not a pointer into the caller's buffer -- callers routinely pass a
// stack-local buffer (e.g. koa's text_render_system.c builds each line into a local char[] per
// entity) that's long gone by flush time, so a stored pointer would dangle and read whatever
// happened to reuse that stack slot in the meantime (observed as garbled/truncated on-screen text:
// only the first character -- whatever was still intact closest to the original write -- rendered
// correctly, with everything after it reading back as stack garbage of unpredictable length/bytes).
#define SKA_FONT_BATCH_ITEM_TEXT_MAX 512
typedef struct FontBatchItem {
    SkaFont* font;
    char text[SKA_FONT_BATCH_ITEM_TEXT_MAX];
    f32 x;
    f32 y;
    f32 scale;
    SkaColor color;
} FontBatchItem;

void renderer_batching_draw_sprites(SpriteBatchItem items[], usize sprite_count);

// Render Layer - Arranges draw order by z index
typedef struct RenderTextureLayer {
    SpriteBatchItem sprite_batch_items[SKA_RENDER_LAYER_BATCH_ITEM_MAX];
    usize sprite_batch_item_count;
} RenderTextureLayer;

typedef struct RenderLayer {
    RenderTextureLayer render_texture_layers[SKA_RENDER_TEXTURE_LAYER_TEXTURE_MAX];
    FontBatchItem font_batch_items[SKA_RENDER_LAYER_FONT_BATCH_ITEM_MAX];
    usize render_texture_layer_count;
    usize font_batch_item_count;
} RenderLayer;

SKA_STATIC_ARRAY_CREATE(RenderLayer, SKA_RENDERER_MAX_Z_INDEX, render_layer_items);
SKA_STATIC_ARRAY_CREATE(int32, SKA_RENDERER_MAX_Z_INDEX, active_render_layer_items_indices);

// Renderer
void ska_renderer_init_2d() {
    // Projection space = the requested resolution (ska_renderer_init sets the render context first)
    const SkaSize2Di resolution = ska_render_context_get()->resolution;
    if (resolution.w > 0 && resolution.h > 0) {
        resolution_width = (f32)resolution.w;
        resolution_height = (f32)resolution.h;
    }
    sprite_renderer_initialize();
    font_renderer_initialize();
}

void ska_renderer_deinit_2d() {
    font_renderer_finalize();
    sprite_renderer_finalize();
}

static inline void update_active_render_layer_index(int32 z_index) {
    const usize size_before = SKA_STATIC_ARRAY_SIZE(active_render_layer_items_indices);
    SKA_STATIC_ARRAY_ADD_IF_UNIQUE(active_render_layer_items_indices, z_index);
    const usize size_after = SKA_STATIC_ARRAY_SIZE(active_render_layer_items_indices);
    if (size_before != size_after) {
        SKA_STATIC_ARRAY_SORT_INT(active_render_layer_items_indices);
    }
}

static inline void ska_renderer_queue_sprite_draw_call(SpriteBatchItem* item, int32 z_index) {
    const int32 array_z_index = ska_math_clamp_int(z_index + SKA_RENDERER_MAX_Z_INDEX / 2, 0, SKA_RENDERER_MAX_Z_INDEX - 1);
    // Get texture layer index for render texture
    usize texture_layer_index = render_layer_items[array_z_index].render_texture_layer_count;
    for (usize i = 0; i < render_layer_items[array_z_index].render_texture_layer_count; i++) {
        if (item->texture == render_layer_items[array_z_index].render_texture_layers[i].sprite_batch_items[0].texture && item->shader_instance == render_layer_items[array_z_index].render_texture_layers[i].sprite_batch_items[0].shader_instance) {
            texture_layer_index = i;
            break;
        }
    }
    RenderTextureLayer* texture_layer =  &render_layer_items[array_z_index].render_texture_layers[texture_layer_index];
    // Increment render texture layer count if first sprite
    if (texture_layer->sprite_batch_item_count == 0) {
        render_layer_items[array_z_index].render_texture_layer_count++;
    }
    // Copy batch item into batch items array and increment item count
    SKA_ASSERT_FMT(texture_layer->sprite_batch_item_count + 1 < SKA_RENDER_LAYER_BATCH_ITEM_MAX, "Exceeded SKA_RENDER_LAYER_BATCH_ITEM_MAX '%d'", SKA_RENDER_LAYER_BATCH_ITEM_MAX);
    SpriteBatchItem* current_item = &texture_layer->sprite_batch_items[texture_layer->sprite_batch_item_count++];
    memcpy(current_item, item, sizeof(SpriteBatchItem));
    // Update active render layer indices
    update_active_render_layer_index(array_z_index);
}

void ska_renderer_queue_sprite_draw(SkaTexture* texture, SkaRect2 source_rect, SkaSize2D dest_size, SkaColor color, bool flip_h, bool flip_v, const SkaTransform2D* transform2_d, int32 z_index, SkaShaderInstance* shader_instance) {
    if (texture == NULL) {
        ska_logger_error("NULL texture, not submitting draw call!");
        return;
    }
    SpriteBatchItem item;
    item.texture = texture;
    item.source_rect = source_rect;
    item.dest_size = dest_size;
    item.color = color;
    item.flip_h = flip_h;
    item.flip_v = flip_v;
    item.transform2_d.model = SKA_MATRIX4_IDENTITY;
    item.shader_instance = shader_instance;
    ska_transform2d_transform_to_mat4(transform2_d, &item.transform2_d.model);
    ska_renderer_queue_sprite_draw_call(&item, z_index);
}

void ska_renderer_queue_sprite_draw2(SkaTexture* texture, SkaRect2 source_rect, SkaSize2D dest_size, SkaColor color, bool flip_h, bool flip_v, SkaMatrix4* trs_matrix, int32 z_index, SkaShaderInstance* shader_instance) {
    if (texture == NULL) {
        ska_logger_error("NULL texture, not submitting draw call!");
        return;
    }
    SpriteBatchItem item;
    item.texture = texture;
    item.source_rect = source_rect;
    item.dest_size = dest_size;
    item.color = color;
    item.flip_h = flip_h;
    item.flip_v = flip_v;
    item.shader_instance = shader_instance;
    ska_matrix4_copy_implace(&item.transform2_d.model, trs_matrix);
    ska_renderer_queue_sprite_draw_call(&item, z_index);
}

void ska_renderer_queue_font_draw_call(SkaFont* font, const char* text, f32 x, f32 y, f32 scale, SkaColor color, int32 z_index) {
    if (font == NULL) {
        ska_logger_error("NULL font, not submitting draw call!");
        return;
    }

    FontBatchItem item;
    item.font = font;
    strncpy(item.text, text, sizeof(item.text) - 1);
    item.text[sizeof(item.text) - 1] = '\0';
    item.x = x;
    item.y = y;
    item.scale = scale;
    item.color = color;
    const int32 array_z_index = ska_math_clamp_int(z_index + SKA_RENDERER_MAX_Z_INDEX / 2, 0, SKA_RENDERER_MAX_Z_INDEX - 1);
    SKA_ASSERT_FMT(render_layer_items[array_z_index].font_batch_item_count < SKA_RENDER_LAYER_FONT_BATCH_ITEM_MAX, "Exceeded SKA_RENDER_LAYER_FONT_BATCH_ITEM_MAX '%d'", SKA_RENDER_LAYER_FONT_BATCH_ITEM_MAX);
    // Update font batch item on render layer
    render_layer_items[array_z_index].font_batch_items[render_layer_items[array_z_index].font_batch_item_count++] = item;
    // Update active render layer indices
    update_active_render_layer_index(array_z_index);
}

void ska_renderer_flush_2d_batches() {
    for (usize i = 0; i < active_render_layer_items_indices_count; i++) {
        const usize layer_index = active_render_layer_items_indices[i];
        // Sprite
        for (usize render_texture_index = 0; render_texture_index < render_layer_items[layer_index].render_texture_layer_count; render_texture_index++) {
            RenderTextureLayer* render_texture_layer = &render_layer_items[layer_index].render_texture_layers[render_texture_index];
            renderer_batching_draw_sprites(render_texture_layer->sprite_batch_items, render_texture_layer->sprite_batch_item_count);
            render_texture_layer->sprite_batch_item_count = 0;
        }
        render_layer_items[layer_index].render_texture_layer_count = 0;
        // Font
        for (usize font_index = 0; font_index < render_layer_items[layer_index].font_batch_item_count; font_index++) {
            font_renderer_draw_text(
                render_layer_items[layer_index].font_batch_items[font_index].font,
                render_layer_items[layer_index].font_batch_items[font_index].text,
                render_layer_items[layer_index].font_batch_items[font_index].x,
                render_layer_items[layer_index].font_batch_items[font_index].y,
                render_layer_items[layer_index].font_batch_items[font_index].scale,
                &render_layer_items[layer_index].font_batch_items[font_index].color
            );
        }
        render_layer_items[layer_index].font_batch_item_count = 0;
    }

    SKA_STATIC_ARRAY_EMPTY(render_layer_items);
    SKA_STATIC_ARRAY_EMPTY(active_render_layer_items_indices);
}


// --- Sprite Renderer --- //
#define VERTS_STRIDE 10
void sprite_renderer_initialize() {
    GLfloat vertices[] = {
        //id (1) // positions (2) // texture coordinates (2) // color (4) // is pixel art (1)
        0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,

        0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f
    };

    // Initialize render data
    glGenVertexArrays(1, &sprite_quad_vao);
    glGenBuffers(1, &sprite_quad_vbo);

    glBindBuffer(GL_ARRAY_BUFFER, sprite_quad_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

    glBindVertexArray(sprite_quad_vao);
    // id attribute
    glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, VERTS_STRIDE * sizeof(GLfloat), (void*) NULL);
    glEnableVertexAttribArray(0);
    // position attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, VERTS_STRIDE * sizeof(GLfloat), (GLvoid*)(sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
    // texture coords attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, VERTS_STRIDE * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);
    // color attribute
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, VERTS_STRIDE * sizeof(GLfloat), (GLvoid*)(5 * sizeof(GLfloat)));
    glEnableVertexAttribArray(3);
    // is pixel art attribute
    glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, VERTS_STRIDE * sizeof(GLfloat), (GLvoid*)(9 * sizeof(GLfloat)));
    glEnableVertexAttribArray(4);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // compile shaders
    sprite_shader = ska_shader_compile_new_shader(ska_opengl_shader_source_vertex_sprite,
                                                 ska_opengl_shader_source_fragment_sprite);
    sprite_renderer_update_resolution();
    ska_renderer_set_sprite_shader_default_params(sprite_shader);
}

void sprite_renderer_finalize() {}

void ska_renderer_set_sprite_shader_default_params(SkaShader* shader) {
    ska_shader_use(shader);
    ska_shader_set_int(shader, "TEXTURE", 0);
    ska_shader_set_mat4_float(shader, "CRE_PROJECTION", &sprite_projection);
}

void sprite_renderer_update_resolution() {
    sprite_projection = ska_matrix4_ortho(0.0f, resolution_width, resolution_height, 0.0f, -1.0f, 1.0f);
}

void renderer_batching_draw_sprites(SpriteBatchItem items[], usize sprite_count) {
// Must match the size of the sprite shader's CRE_MODELS[] uniform array (shader_source.h); a sprite
// indexed past it gets no model matrix and silently doesn't render, so larger batches are drawn in chunks.
#define MAX_SPRITES_PER_DRAW 100
#define NUMBER_OF_VERTICES 6
#define VERTEX_BUFFER_SIZE (VERTS_STRIDE * NUMBER_OF_VERTICES * MAX_SPRITES_PER_DRAW)

    if (sprite_count <= 0) {
        return;
    }

    glDepthMask(false);

    glBindVertexArray(sprite_quad_vao);
    glBindBuffer(GL_ARRAY_BUFFER, sprite_quad_vbo);

    SkaTexture* texture = items[0].texture;

    GLfloat verts[VERTEX_BUFFER_SIZE];
    for (usize chunk_start = 0; chunk_start < sprite_count; chunk_start += MAX_SPRITES_PER_DRAW) {
        const usize chunk_count = SKA_MATH_MIN(sprite_count - chunk_start, (usize)MAX_SPRITES_PER_DRAW);
        for (usize n = 0; n < chunk_count; n++) {
            SpriteBatchItem* item = &items[chunk_start + n];
            if (item->shader_instance != NULL) {
                ska_shader_use(item->shader_instance->shader);
                ska_renderer_set_shader_instance_params(item->shader_instance);
            } else {
                ska_shader_use(sprite_shader);
            }

            ska_matrix4_scale_and_mul(&item->transform2_d.model, (SkaVector3) {
                item->dest_size.w, item->dest_size.h, 1.0f
            });
            const f32 sprite_id = (f32) n;
            const f32 determinate = ska_matrix4_determinant(&item->transform2_d.model);
            const SkaTextureCoordinates texture_coords = renderer_get_texture_coordinates(texture, &item->source_rect, item->flip_h, item->flip_v);
            // concat CRE_MODELS[] string for uniform param
            char models_buffer[24];
            sprintf(models_buffer, "CRE_MODELS[%zu]", n);
            ska_shader_set_mat4_float(sprite_shader, models_buffer, &item->transform2_d.model);

            // Loop over vertices
            for (int32 j = 0; j < NUMBER_OF_VERTICES; j++) {
                bool is_s_min;
                bool is_t_min;
                if (determinate >= 0.0f) {
                    is_s_min = j == 0 || j == 2 || j == 3;
                    is_t_min = j == 1 || j == 2 || j == 5;
                } else {
                    is_s_min = j == 1 || j == 2 || j == 5;
                    is_t_min = j == 0 || j == 2 || j == 3;
                }
                const int32 row = (j * VERTS_STRIDE) + ((int32) n * (VERTS_STRIDE * NUMBER_OF_VERTICES));
                verts[row + 0] = sprite_id;
                verts[row + 1] = is_s_min ? 0.0f : 1.0f;
                verts[row + 2] = is_t_min ? 0.0f : 1.0f;
                verts[row + 3] = is_s_min ? texture_coords.s_min : texture_coords.s_max;
                verts[row + 4] = is_t_min ? texture_coords.t_min : texture_coords.t_max;
                verts[row + 5] = item->color.r;
                verts[row + 6] = item->color.g;
                verts[row + 7] = item->color.b;
                verts[row + 8] = item->color.a;
                verts[row + 9] = (f32)texture->apply_nearest_neighbor;
            }
        }

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture->id);

        glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr) (sizeof(GLfloat) * VERTS_STRIDE * NUMBER_OF_VERTICES * chunk_count), verts, GL_DYNAMIC_DRAW);
        glDrawArrays(GL_TRIANGLES, 0, (GLsizei) (chunk_count * NUMBER_OF_VERTICES));

        ska_renderer_print_opengl_errors();
    }

    glBindVertexArray(0);
    glDepthMask(true);

#undef MAX_SPRITES_PER_DRAW
#undef NUMBER_OF_VERTICES
#undef VERTEX_BUFFER_SIZE
}
#undef VERTS_STRIDE

// --- Font Renderer --- //
void font_renderer_initialize() {
  if (FT_Init_FreeType(&ska_render_context_get()->free_type_library)) {
    ska_logger_error("Unable to initialize FreeType library!");
  }
    font_shader = ska_shader_compile_new_shader(ska_opengl_shader_source_vertex_font,
                                               ska_opengl_shader_source_fragment_font);
    font_renderer_update_resolution();
}

void font_renderer_finalize() {
  FT_Done_FreeType(ska_render_context_get()->free_type_library);
}

void font_renderer_update_resolution() {
    SkaMatrix4 proj = ska_matrix4_ortho(0.0f, resolution_width, resolution_height, 0.0f, -1.0f, 1.0f);
    ska_shader_use(font_shader);
    ska_shader_set_mat4_float(font_shader, "projection", &proj);
}

void font_renderer_draw_text(const SkaFont* font, const char* text, f32 x, f32 y, f32 scale, const SkaColor* color) {
    SkaVector2 current_scale = {scale, scale };
    ska_shader_use(font_shader);
    ska_shader_set_vec4_float(font_shader, "textColor", color->r, color->g, color->b, color->a);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(font->vao);

    // Iterate through all characters
    const char* c = text;
    const usize text_length = strlen(text);
    for (usize i = 0; i < text_length; i++, ++c) {
        const SkaFontCharacter* glyph = ska_font_get_glyph(font, (unsigned char)*c);
        if (!glyph) continue;
        SkaFontCharacter ch = *glyph;
        const f32 x_pos = x + (ch.bearing.x * current_scale.x);
        const f32 w = ch.size.x * current_scale.x;
        const f32 h = ch.size.y * current_scale.y;
        // (x, y) is top-left of the text line, matching the sprite renderer's convention.
        const f32 top_y = y + (font->ascent - ch.bearing.y) * current_scale.y;
        const f32 bottom_y = top_y + h;
        // Update VBO for each characters
        GLfloat verts[6][4] = {
            {x_pos,     top_y,    0.0f, 0.0f},
            {x_pos,     bottom_y, 0.0f, 1.0f},
            {x_pos + w, bottom_y, 1.0f, 1.0f},

            {x_pos,     top_y,    0.0f, 0.0f},
            {x_pos + w, bottom_y, 1.0f, 1.0f},
            {x_pos + w, top_y,    1.0f, 0.0f}
        };
        // Render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.texture_id);
        // Update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, font->vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(verts), verts);
        // Render
        glDrawArrays(GL_TRIANGLES, 0, 6);
        x += (f32) (ch.advance >> 6) * current_scale.x; // bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
    }
    // Unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

// --- Misc --- //
SkaTextureCoordinates renderer_get_texture_coordinates(const SkaTexture* texture, const SkaRect2* draw_source, bool flip_h, bool flip_v) {
    GLfloat s_min = 0.0f;
    GLfloat s_max = 1.0f;
    GLfloat t_min = 0.0f;
    GLfloat t_max = 1.0f;
    // S
    if (texture->width != (GLsizei)draw_source->w || texture->height != (GLsizei)draw_source->h) {
        s_min = (draw_source->x + 0.5f) / (f32) texture->width;
        s_max = (draw_source->x + draw_source->w - 0.5f) / (f32) texture->width;
        t_min = (draw_source->y + 0.5f) / (f32) texture->height;
        t_max = (draw_source->y + draw_source->h - 0.5f) / (f32) texture->height;
    }
    if (flip_h) {
        const GLfloat temp_s_min = s_min;
        s_min = s_max;
        s_max = temp_s_min;
    }
    if (flip_v) {
        const GLfloat temp_t_min = t_min;
        t_min = t_max;
        t_max = temp_t_min;
    }
    return (SkaTextureCoordinates) {
        s_min, s_max, t_min, t_max
    };
}

#endif // #if SKA_RENDERING
