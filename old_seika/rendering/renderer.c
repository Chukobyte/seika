#include "renderer.h"

#include <stdlib.h>
#include <stddef.h>

#include "render_context.h"
#include "shader/shader.h"
#include "shader/shader_cache.h"
#include "shader/shader_source.h"
#include "seika/assert.h"
#include "seika/logger.h"
#include "seika/data_structures/static_array.h"

#define SKA_RENDER_TO_FRAMEBUFFER
#define SKA_RENDER_LAYER_BATCH_ITEM_MAX 1024
#define SKA_RENDER_LAYER_FONT_BATCH_ITEM_MAX 100
#define SKA_RENDER_TEXTURE_LAYER_TEXTURE_MAX 64

#ifdef SKA_RENDER_TO_FRAMEBUFFER
#include "frame_buffer.h"
#endif

typedef struct SkaTextureCoordinates {
    GLfloat sMin;
    GLfloat sMax;
    GLfloat tMin;
    GLfloat tMax;
} SkaTextureCoordinates;

static SkaTextureCoordinates renderer_get_texture_coordinates(const SkaTexture* texture, const SkaRect2* drawSource, bool flipH, bool flipV);
static void renderer_set_shader_instance_params(SkaShaderInstance* shaderInstance);
static void renderer_print_opengl_errors();

static void sprite_renderer_initialize();
static void sprite_renderer_finalize();
static void sprite_renderer_update_resolution();

static void font_renderer_initialize();
static void font_renderer_finalize();
static void font_renderer_update_resolution();
static void font_renderer_draw_text(const SkaFont* font, const char* text, f32 x, f32 y, f32 scale, const SkaColor* color);

static GLuint spriteQuadVAO;
static GLuint spriteQuadVBO;

static SkaShader* spriteShader = NULL;
static SkaShader* fontShader = NULL;

// Global Shader Params
static f32 globalShaderParamTime = 0.0f;

static f32 resolutionWidth = 800.0f;
static f32 resolutionHeight = 600.0f;
static mat4 spriteProjection = {
    {1.0f, 0.0f, 0.0f, 0.0f},
    {0.0f, 1.0f, 0.0f, 0.0f},
    {0.0f, 0.0f, 1.0f, 0.0f},
    {0.0f, 0.0f, 0.0f, 1.0f}
};

// Sprite Batching
typedef struct SpriteBatchItem {
    SkaTexture* texture;
    SkaRect2 sourceRect;
    SkaSize2D destSize;
    SkaColor color;
    bool flipH;
    bool flipV;
    SkaRendererTransform2D transform2D;
    SkaShaderInstance* shaderInstance;
} SpriteBatchItem;

typedef struct FontBatchItem {
    SkaFont* font;
    const char* text;
    f32 x;
    f32 y;
    f32 scale;
    SkaColor color;
} FontBatchItem;

void renderer_batching_draw_sprites(SpriteBatchItem items[], usize spriteCount);

// Render Layer - Arranges draw order by z index
typedef struct RenderTextureLayer {
    SpriteBatchItem spriteBatchItems[SKA_RENDER_LAYER_BATCH_ITEM_MAX];
    usize spriteBatchItemCount;
} RenderTextureLayer;

typedef struct RenderLayer {
    RenderTextureLayer renderTextureLayers[SKA_RENDER_TEXTURE_LAYER_TEXTURE_MAX];
    FontBatchItem fontBatchItems[SKA_RENDER_LAYER_FONT_BATCH_ITEM_MAX];
    usize renderTextureLayerCount;
    usize fontBatchItemCount;
} RenderLayer;

SKA_STATIC_ARRAY_CREATE(RenderLayer, SKA_RENDERER_MAX_Z_INDEX, render_layer_items);
SKA_STATIC_ARRAY_CREATE(int32, SKA_RENDERER_MAX_Z_INDEX, active_render_layer_items_indices);

// Renderer
void ska_renderer_initialize(int32 inWindowWidth, int32 inWindowHeight, int32 inResolutionWidth, int32 inResolutionHeight, bool maintainAspectRatio) {
    resolutionWidth = (f32)inResolutionWidth;
    resolutionHeight = (f32)inResolutionHeight;
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glDisable(GL_MULTISAMPLE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    ska_render_context_initialize();
    ska_renderer_update_window_size(inWindowWidth, inWindowHeight);
    sprite_renderer_initialize();
    font_renderer_initialize();
    ska_shader_cache_initialize();
#ifdef SKA_RENDER_TO_FRAMEBUFFER
    // Initialize framebuffer
    SKA_ASSERT_FMT(ska_frame_buffer_initialize(inWindowWidth, inWindowHeight, inResolutionWidth, inResolutionHeight), "Framebuffer didn't initialize!");
    ska_frame_buffer_set_maintain_aspect_ratio(maintainAspectRatio);
#endif
    // Set initial data for render layer
    for (usize i = 0; i < SKA_RENDERER_MAX_Z_INDEX; i++) {
        render_layer_items[i].renderTextureLayerCount = 0;
        render_layer_items[i].fontBatchItemCount = 0;
        for (usize j = 0; j < SKA_RENDER_TEXTURE_LAYER_TEXTURE_MAX; j++) {
            render_layer_items[i].renderTextureLayers[j].spriteBatchItemCount = 0;
        }
    }
}

void ska_renderer_finalize() {
    font_renderer_finalize();
    sprite_renderer_finalize();
    ska_render_context_finalize();
    ska_shader_cache_finalize();
#ifdef SKA_RENDER_TO_FRAMEBUFFER
    ska_frame_buffer_finalize();
#endif
}

void ska_renderer_update_window_size(int32 windowWidth, int32 windowHeight) {
#ifdef SKA_RENDER_TO_FRAMEBUFFER
    const SkaFrameBufferViewportData data = ska_frame_buffer_generate_viewport_data(windowWidth, windowHeight);
#else
    struct ViewportData {
        SKAVector2i position;
        SKASize2Di size;
    };
    const struct ViewportData data = { .position = { .x = 0, .y = 0 }, .size = { .w = windowWidth, .h = windowHeight } };
#endif
    glViewport(data.position.x, data.position.y, data.size.w, data.size.h);
    SkaRenderContext* renderContext = ska_render_context_get();
    renderContext->windowWidth = data.size.w;
    renderContext->windowHeight = data.size.h;
#ifdef SKA_RENDER_TO_FRAMEBUFFER
    ska_frame_buffer_resize_texture(data.size.w, data.size.h);
#endif
}

static inline void update_active_render_layer_index(int32 zIndex) {
    const usize sizeBefore = SKA_STATIC_ARRAY_SIZE(active_render_layer_items_indices);
    SKA_STATIC_ARRAY_ADD_IF_UNIQUE(active_render_layer_items_indices, zIndex);
    const usize sizeAfter = SKA_STATIC_ARRAY_SIZE(active_render_layer_items_indices);
    if (sizeBefore != sizeAfter) {
        SKA_STATIC_ARRAY_SORT_INT(active_render_layer_items_indices);
    }
}

static inline void ska_renderer_queue_sprite_draw_call(SpriteBatchItem* item, int32 zIndex) {
    const int32 arrayZIndex = ska_math_clamp_int(zIndex + SKA_RENDERER_MAX_Z_INDEX / 2, 0, SKA_RENDERER_MAX_Z_INDEX - 1);
    // Get texture layer index for render texture
    usize textureLayerIndex = render_layer_items[arrayZIndex].renderTextureLayerCount;
    for (usize i = 0; i < render_layer_items[arrayZIndex].renderTextureLayerCount; i++) {
        if (item->texture == render_layer_items[arrayZIndex].renderTextureLayers[i].spriteBatchItems[0].texture && item->shaderInstance == render_layer_items[arrayZIndex].renderTextureLayers[i].spriteBatchItems[0].shaderInstance) {
            textureLayerIndex = i;
            break;
        }
    }
    RenderTextureLayer* textureLayer =  &render_layer_items[arrayZIndex].renderTextureLayers[textureLayerIndex];
    // Increment render texture layer count if first sprite
    if (textureLayer->spriteBatchItemCount == 0) {
        render_layer_items[arrayZIndex].renderTextureLayerCount++;
    }
    // Copy batch item into batch items array and increment item count
    SKA_ASSERT_FMT(textureLayer->spriteBatchItemCount + 1 < SKA_RENDER_LAYER_BATCH_ITEM_MAX, "Exceeded SKA_RENDER_LAYER_BATCH_ITEM_MAX '%d'", SKA_RENDER_LAYER_BATCH_ITEM_MAX);
    SpriteBatchItem* currentItem = &textureLayer->spriteBatchItems[textureLayer->spriteBatchItemCount++];
    memcpy(currentItem, item, sizeof(SpriteBatchItem));
    // Update active render layer indices
    update_active_render_layer_index(arrayZIndex);
}

void ska_renderer_queue_sprite_draw(SkaTexture* texture, SkaRect2 sourceRect, SkaSize2D destSize, SkaColor color, bool flipH, bool flipV, const SkaTransform2D* transform2D, int32 zIndex, SkaShaderInstance* shaderInstance) {
    if (texture == NULL) {
        ska_logger_error("NULL texture, not submitting draw call!");
        return;
    }
    SpriteBatchItem item = (SpriteBatchItem){ .texture = texture, .sourceRect = sourceRect, .destSize = destSize, .color = color, .flipH = flipH, .flipV = flipV, .transform2D = { .model = {{0}} }, .shaderInstance = shaderInstance };
    ska_transform2d_transform_to_mat4(transform2D, item.transform2D.model);
    ska_renderer_queue_sprite_draw_call(&item, zIndex);
}

void ska_renderer_queue_sprite_draw2(SkaTexture* texture, SkaRect2 sourceRect, SkaSize2D destSize, SkaColor color, bool flipH, bool flipV, mat4 trsMatrix, int32 zIndex, SkaShaderInstance* shaderInstance) {
    if (texture == NULL) {
        ska_logger_error("NULL texture, not submitting draw call!");
        return;
    }
    SpriteBatchItem item = (SpriteBatchItem){ .texture = texture, .sourceRect = sourceRect, .destSize = destSize, .color = color, .flipH = flipH, .flipV = flipV, .transform2D = { .model = {{0}} }, .shaderInstance = shaderInstance };
    glm_mat4_copy(trsMatrix, item.transform2D.model);
    ska_renderer_queue_sprite_draw_call(&item, zIndex);
}

void ska_renderer_queue_font_draw_call(SkaFont* font, const char* text, f32 x, f32 y, f32 scale, SkaColor color, int32 zIndex) {
    if (font == NULL) {
        ska_logger_error("NULL font, not submitting draw call!");
        return;
    }

    FontBatchItem item = { .font = font, .text = text, .x = x, .y = y, .scale = scale, .color = color };
    const int32 arrayZIndex = ska_math_clamp_int(zIndex + SKA_RENDERER_MAX_Z_INDEX / 2, 0, SKA_RENDERER_MAX_Z_INDEX - 1);
    // Update font batch item on render layer
    render_layer_items[arrayZIndex].fontBatchItems[render_layer_items[arrayZIndex].fontBatchItemCount++] = item;
    // Update active render layer indices
    update_active_render_layer_index(arrayZIndex);
}

static void ska_renderer_flush_batches() {
    for (usize i = 0; i < active_render_layer_items_indices_count; i++) {
        const usize layerIndex = active_render_layer_items_indices[i];
        // Sprite
        for (usize renderTextureIndex = 0; renderTextureIndex < render_layer_items[layerIndex].renderTextureLayerCount; renderTextureIndex++) {
            RenderTextureLayer* renderTextureLayer = &render_layer_items[layerIndex].renderTextureLayers[renderTextureIndex];
            renderer_batching_draw_sprites(renderTextureLayer->spriteBatchItems, renderTextureLayer->spriteBatchItemCount);
            renderTextureLayer->spriteBatchItemCount = 0;
        }
        render_layer_items[layerIndex].renderTextureLayerCount = 0;
        // Font
        for (usize fontIndex = 0; fontIndex < render_layer_items[layerIndex].fontBatchItemCount; fontIndex++) {
            font_renderer_draw_text(
                render_layer_items[layerIndex].fontBatchItems[fontIndex].font,
                render_layer_items[layerIndex].fontBatchItems[fontIndex].text,
                render_layer_items[layerIndex].fontBatchItems[fontIndex].x,
                render_layer_items[layerIndex].fontBatchItems[fontIndex].y,
                render_layer_items[layerIndex].fontBatchItems[fontIndex].scale,
                &render_layer_items[layerIndex].fontBatchItems[fontIndex].color
            );
        }
        render_layer_items[layerIndex].fontBatchItemCount = 0;
    }

    SKA_STATIC_ARRAY_EMPTY(render_layer_items);
    SKA_STATIC_ARRAY_EMPTY(active_render_layer_items_indices);
}

void ska_renderer_process_and_flush_batches(const SkaColor* backgroundColor) {
#ifdef SKA_RENDER_TO_FRAMEBUFFER
    ska_frame_buffer_bind();
#endif

    // Clear framebuffer with background color
    glClearColor(backgroundColor->r, backgroundColor->g, backgroundColor->b, backgroundColor->a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

#ifdef SKA_RENDER_TO_FRAMEBUFFER
    SkaFrameBufferViewportData* viewportData = ska_frame_buffer_get_cached_viewport_data();
    glViewport(0, 0, viewportData->size.w, viewportData->size.h);
#endif

    ska_renderer_flush_batches();

#ifdef SKA_RENDER_TO_FRAMEBUFFER
    ska_frame_buffer_unbind();

    // Clear screen texture background
    static const SkaColor screenBackgroundColor = {0.0f, 0.0f, 0.0f, 1.0f };
    glClearColor(screenBackgroundColor.r, screenBackgroundColor.g, screenBackgroundColor.b, screenBackgroundColor.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // Draw screen texture from framebuffer
    SkaShaderInstance* screenShaderInstance = ska_frame_buffer_get_screen_shader();
    ska_shader_use(screenShaderInstance->shader);

    // Apply shader instance params
    renderer_set_shader_instance_params(screenShaderInstance);

    glBindVertexArray(ska_frame_buffer_get_quad_vao());
    glViewport(viewportData->position.x, viewportData->position.y, viewportData->size.w, viewportData->size.h);

    glBindTexture(GL_TEXTURE_2D, ska_frame_buffer_get_color_buffer_texture());	// use the color attachment texture as the texture of the quad plane
    glDrawArrays(GL_TRIANGLES, 0, 6);
#endif
}

#ifdef SKA_RENDER_TO_FRAMEBUFFER
void ska_renderer_process_and_flush_batches_just_framebuffer(const SkaColor *backgroundColor) {
    ska_frame_buffer_bind();

    // Clear framebuffer with background color
    glClearColor(backgroundColor->r, backgroundColor->g, backgroundColor->b, backgroundColor->a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    ska_renderer_flush_batches();

    ska_frame_buffer_unbind();
}
#endif

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
    glGenVertexArrays(1, &spriteQuadVAO);
    glGenBuffers(1, &spriteQuadVBO);

    glBindBuffer(GL_ARRAY_BUFFER, spriteQuadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

    glBindVertexArray(spriteQuadVAO);
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
    spriteShader = ska_shader_compile_new_shader(SKA_OPENGL_SHADER_SOURCE_VERTEX_SPRITE,
                                                 SKA_OPENGL_SHADER_SOURCE_FRAGMENT_SPRITE);
    sprite_renderer_update_resolution();
    ska_renderer_set_sprite_shader_default_params(spriteShader);
}

void sprite_renderer_finalize() {}

void ska_renderer_set_sprite_shader_default_params(SkaShader* shader) {
    ska_shader_use(shader);
    ska_shader_set_int(shader, "TEXTURE", 0);
    ska_shader_set_mat4_float(shader, "CRE_PROJECTION", &spriteProjection);
}

void sprite_renderer_update_resolution() {
    glm_mat4_identity(spriteProjection);
    glm_ortho(0.0f, resolutionWidth, resolutionHeight, 0.0f, -1.0f, 1.0f, spriteProjection);
}

void renderer_batching_draw_sprites(SpriteBatchItem items[], usize spriteCount) {
#define MAX_SPRITE_COUNT 2000
#define NUMBER_OF_VERTICES 6
#define VERTEX_BUFFER_SIZE (VERTS_STRIDE * NUMBER_OF_VERTICES * MAX_SPRITE_COUNT)

    if (spriteCount <= 0) {
        return;
    }

    SKA_ASSERT(spriteCount <= MAX_SPRITE_COUNT);

    glDepthMask(false);

    glBindVertexArray(spriteQuadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, spriteQuadVBO);

    SkaTexture* texture = items[0].texture;

    GLfloat verts[VERTEX_BUFFER_SIZE];
    for (usize i = 0; i < spriteCount; i++) {
        if (items[i].shaderInstance != NULL) {
            ska_shader_use(items[i].shaderInstance->shader);
            renderer_set_shader_instance_params(items[i].shaderInstance);
        } else {
            ska_shader_use(spriteShader);
        }

        glm_scale(items[i].transform2D.model, (vec3) {
            items[i].destSize.w, items[i].destSize.h, 1.0f
        });
        const f32 spriteId = (f32) i;
        const f32 determinate = glm_mat4_det(items[i].transform2D.model);
        const SkaTextureCoordinates textureCoords = renderer_get_texture_coordinates(texture, &items[i].sourceRect, items[i].flipH, items[i].flipV);
        // concat CRE_MODELS[] string for uniform param
        char modelsBuffer[24];
        sprintf(modelsBuffer, "CRE_MODELS[%zu]", i);
        ska_shader_set_mat4_float(spriteShader, modelsBuffer, &items[i].transform2D.model);

        // Loop over vertices
        for (int32 j = 0; j < NUMBER_OF_VERTICES; j++) {
            bool isSMin;
            bool isTMin;
            if (determinate >= 0.0f) {
                isSMin = j == 0 || j == 2 || j == 3;
                isTMin = j == 1 || j == 2 || j == 5;
            } else {
                isSMin = j == 1 || j == 2 || j == 5;
                isTMin = j == 0 || j == 2 || j == 3;
            }
            const int32 row = (j * VERTS_STRIDE) + ((int32) i * (VERTS_STRIDE * NUMBER_OF_VERTICES));
            verts[row + 0] = spriteId;
            verts[row + 1] = isSMin ? 0.0f : 1.0f;
            verts[row + 2] = isTMin ? 0.0f : 1.0f;
            verts[row + 3] = isSMin ? textureCoords.sMin : textureCoords.sMax;
            verts[row + 4] = isTMin ? textureCoords.tMin : textureCoords.tMax;
            verts[row + 5] = items[i].color.r;
            verts[row + 6] = items[i].color.g;
            verts[row + 7] = items[i].color.b;
            verts[row + 8] = items[i].color.a;
            verts[row + 9] = (f32)texture->applyNearestNeighbor;
        }
    }

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture->id);

    glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr) sizeof(verts), verts, GL_DYNAMIC_DRAW);
    glDrawArrays(GL_TRIANGLES, 0, (GLsizei) (spriteCount * NUMBER_OF_VERTICES));

    renderer_print_opengl_errors();

    glBindVertexArray(0);
    glDepthMask(true);

#undef MAX_SPRITE_COUNT
#undef NUMBER_OF_VERTICES
#undef VERTEX_BUFFER_SIZE
}
#undef VERTS_STRIDE

// --- Font Renderer --- //
void font_renderer_initialize() {
    if (FT_Init_FreeType(&ska_render_context_get()->freeTypeLibrary)) {
        ska_logger_error("Unable to initialize FreeType library!");
    }
    fontShader = ska_shader_compile_new_shader(SKA_OPENGL_SHADER_SOURCE_VERTEX_FONT,
                                               SKA_OPENGL_SHADER_SOURCE_FRAGMENT_FONT);
    font_renderer_update_resolution();
}

void font_renderer_finalize() {
    FT_Done_FreeType(ska_render_context_get()->freeTypeLibrary);
}

void font_renderer_update_resolution() {
    mat4 proj = {
        {1.0f, 0.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 1.0f, 0.0f},
        {0.0f, 0.0f, 0.0f, 1.0f}
    };
    glm_ortho(0.0f, resolutionWidth, -resolutionHeight, 0.0f, -1.0f, 1.0f, proj);
    ska_shader_use(fontShader);
    ska_shader_set_mat4_float(fontShader, "projection", &proj);
}

void font_renderer_draw_text(const SkaFont* font, const char* text, f32 x, f32 y, f32 scale, const SkaColor* color) {
    SkaVector2 currentScale = {scale, scale };
    ska_shader_use(fontShader);
    ska_shader_set_vec4_float(fontShader, "textColor", color->r, color->g, color->b, color->a);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(font->VAO);

    // Iterate through all characters
    char* c = (char*) &text[0];
    const usize textLength = strlen(text);
    for (usize i = 0; i < textLength; i++) {
        SkaFontCharacter ch = font->characters[(int32) *c];
        const f32 xPos = x + (ch.bearing.x * currentScale.x);
        const f32 yPos = -y - (ch.size.y - ch.bearing.y) * currentScale.x; // Invert Y because orthographic projection is flipped
        const f32 w = ch.size.x * currentScale.x;
        const f32 h = ch.size.y * currentScale.y;
        // Update VBO for each characters
        GLfloat verts[6][4] = {
            {xPos,     yPos + h, 0.0f, 0.0f},
            {xPos,     yPos,     0.0f, 1.0f},
            {xPos + w, yPos,     1.0f, 1.0f},

            {xPos,     yPos + h, 0.0f, 0.0f},
            {xPos + w, yPos,     1.0f, 1.0f},
            {xPos + w, yPos + h, 1.0f, 0.0f}
        };
        // Render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.textureId);
        // Update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, font->VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(verts), verts);
        // Render
        glDrawArrays(GL_TRIANGLES, 0, 6);
        x += (f32) (ch.advance >> 6) * currentScale.x; // bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
        ++c;
    }
    // Unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

// --- Misc --- //
SkaTextureCoordinates renderer_get_texture_coordinates(const SkaTexture* texture, const SkaRect2* drawSource, bool flipH, bool flipV) {
    GLfloat sMin = 0.0f;
    GLfloat sMax = 1.0f;
    GLfloat tMin = 0.0f;
    GLfloat tMax = 1.0f;
    // S
    if (texture->width != (GLsizei)drawSource->w || texture->height != (GLsizei)drawSource->h) {
        sMin = (drawSource->x + 0.5f) / (f32) texture->width;
        sMax = (drawSource->x + drawSource->w - 0.5f) / (f32) texture->width;
        tMin = (drawSource->y + 0.5f) / (f32) texture->height;
        tMax = (drawSource->y + drawSource->h - 0.5f) / (f32) texture->height;
    }
    if (flipH) {
        const GLfloat tempSMin = sMin;
        sMin = sMax;
        sMax = tempSMin;
    }
    if (flipV) {
        const GLfloat tempTMin = tMin;
        tMin = tMax;
        tMax = tempTMin;
    }
    return (SkaTextureCoordinates) {
        sMin, sMax, tMin, tMax
    };
}

void renderer_set_shader_instance_params(SkaShaderInstance* shaderInstance) {
    // Set global shader params first
    ska_shader_set_float(shaderInstance->shader, "TIME", globalShaderParamTime);

    // Now set shader params specific to the shader instance
    if (shaderInstance->paramsDirty && shaderInstance->paramMap->size > 0) {
        SKA_STRING_HASH_MAP_FOR_EACH(shaderInstance->paramMap, iter) {
            SkaStringHashMapNode* node = iter.pair;
            SkaShaderParam* param = (SkaShaderParam*) node->value;
            switch (param->type) {
            case SkaShaderParamType_BOOL: {
                ska_shader_set_bool(shaderInstance->shader, param->name, param->value.boolValue);
                break;
            }
            case SkaShaderParamType_INT: {
                ska_shader_set_int(shaderInstance->shader, param->name, param->value.intValue);
                break;
            }
            case SkaShaderParamType_FLOAT: {
                ska_shader_set_float(shaderInstance->shader, param->name, param->value.floatValue);
                break;
            }
            case SkaShaderParamType_FLOAT2: {
                ska_shader_set_vec2_float(shaderInstance->shader, param->name, param->value.float2Value.x,param->value.float2Value.y);
                break;
            }
            case SkaShaderParamType_FLOAT3: {
                ska_shader_set_vec3_float(shaderInstance->shader, param->name, param->value.float3Value.x,param->value.float3Value.y, param->value.float3Value.z);
                break;
            }
            case SkaShaderParamType_FLOAT4: {
                ska_shader_set_vec4_float(shaderInstance->shader, param->name, param->value.float4Value.x,param->value.float4Value.y, param->value.float4Value.z,param->value.float4Value.w);
                break;
            }
            }
        }
        shaderInstance->paramsDirty = false;
    }
}

void renderer_print_opengl_errors() {
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        printf("err = %d\n", err);
        switch (err) {
        case GL_NO_ERROR:
            printf("GL_NO_ERROR\n");
            break;
        case GL_INVALID_ENUM:
            printf("GL_INVALID_ENUM\n");
            break;
        case GL_INVALID_VALUE:
            printf("GL_INVALID_VALUE\n");
            break;
        case GL_INVALID_OPERATION:
            printf("GL_INVALID_OPERATION\n");
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            printf("GL_INVALID_FRAMEBUFFER_OPERATION\n");
            break;
        default:
            printf("default\n");
            break;
        }
    }
}

// Shader param stuff
void ska_renderer_set_global_shader_param_time(f32 timeValue) {
    globalShaderParamTime = timeValue;
}
