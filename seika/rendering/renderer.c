#if SKA_RENDERING

// seika/window/window.h pulls in windows.h on Windows; must precede renderer.h
// (which transitively includes glad.h via font.h), since glad's APIENTRY
// definition isn't itself guarded against windows.h's unconditional
// (re)definition (C4005).
#include "seika/window/window.h"

#include "renderer.h"

#include <stdlib.h>

#include "render_context.h"
#include "shader/shader.h"
#include "shader/shader_cache.h"
#include "shader/shader_source.h"
#include "seika/assert.h"
#include "../logger.h"
#include "seika/data_structures/static_array.h"

// TODO: Fix render to framebuffer for 3D
#define SKA_RENDER_TO_FRAMEBUFFER

#ifdef SKA_RENDER_TO_FRAMEBUFFER
#include "frame_buffer.h"
#endif

// Global Shader Params
static f32 global_shader_param_time = 0.0f;

// Renderer
void ska_renderer_init(SkaRendererInitProps props) {
    // glEnable(GL_CULL_FACE); // TODO: Used in 2D, we can flip state for 2D and 3D as needed...
    glEnable(GL_BLEND);
#if !defined(PLATFORM_ANDROID) && !defined(PLATFORM_EMSCRIPTEN)
    // GL_MULTISAMPLE doesn't exist in GLES -- there multisampling is controlled via the EGL
    // config's sample buffers at context-creation time (window_android.c/window_emscripten.c
    // request none), not a runtime glEnable/glDisable toggle, so there's nothing to disable here.
    glDisable(GL_MULTISAMPLE);
#endif
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    ska_render_context_initialize((SkaSize2Di){ props.resolution_width, props.resolution_height });
    ska_renderer_update_window_size(props.window_width, props.window_height);
    ska_shader_cache_initialize();
    if (props.init_2d) {
        ska_renderer_init_2d();
    }
    if (props.init_props_3d) {
      ska_renderer_init_3d((SkaRenderer3DInitProps){
          .fov = props.init_props_3d->fov,
          .near_plane = props.init_props_3d->near_plane,
          .far_plane = props.init_props_3d->far_plane});
    }
#ifdef SKA_RENDER_TO_FRAMEBUFFER
    // Initialize framebuffer
    SKA_ASSERT_FMT(ska_frame_buffer_initialize(props.window_width, props.window_height, props.resolution_width, props.resolution_height), "Framebuffer didn't initialize!");
    ska_frame_buffer_set_maintain_aspect_ratio(props.maintain_aspect_ratio);
#endif
}

void ska_renderer_deinit() {
    ska_render_context_finalize();
    ska_shader_cache_finalize();
#ifdef SKA_RENDER_TO_FRAMEBUFFER
    ska_frame_buffer_finalize();
#endif
}

void ska_renderer_clear(f32 red, f32 green, f32 blue, f32 alpha) {
    glClearColor(red, green, blue, alpha);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void ska_renderer_update_window_size(int32 window_width, int32 window_height) {
#ifdef SKA_RENDER_TO_FRAMEBUFFER
    const SkaFrameBufferViewportData data = ska_frame_buffer_generate_viewport_data(window_width, window_height);
#else
    struct ViewportData {
        SkaVector2i position;
        SkaSize2Di size;
    };
    const struct ViewportData data = { .position = { .x = 0, .y = 0 }, .size = { .w = windowWidth, .h = windowHeight } };
#endif
    glViewport(data.position.x, data.position.y, data.size.w, data.size.h);
    SkaWindow* window = ska_win_get_primary_window();
    SKA_ASSERT_FMT(window, "Should have a primary window!");
    window->width = data.size.w;
    window->height = data.size.h;
#ifdef SKA_RENDER_TO_FRAMEBUFFER
    ska_frame_buffer_resize_texture(data.size.w, data.size.h);
#endif
}

static void ska_renderer_flush_batches() {
    ska_renderer_flush_2d_batches();
    ska_renderer_flush_meshes();
    ska_renderer_flush_lights();
    ska_renderer_flush_debug_lines();
}

void ska_renderer_process_and_flush_batches(const SkaColor* background_color) {
#ifdef SKA_RENDER_TO_FRAMEBUFFER
    ska_frame_buffer_bind();
#endif

    // Clear framebuffer with background color
    ska_renderer_clear(background_color->r, background_color->g, background_color->b, background_color->a);

#ifdef SKA_RENDER_TO_FRAMEBUFFER
    SkaFrameBufferViewportData* viewport_data = ska_frame_buffer_get_cached_viewport_data();
    glViewport(0, 0, viewport_data->size.w, viewport_data->size.h);
#endif

    ska_renderer_flush_batches();

#ifdef SKA_RENDER_TO_FRAMEBUFFER
    ska_frame_buffer_unbind();

    // Clear screen texture background
    static const SkaColor screen_background_color = {0.0f, 0.0f, 0.0f, 1.0f };
    ska_renderer_clear(screen_background_color.r, screen_background_color.g, screen_background_color.b, screen_background_color.a);
    // Draw screen texture from framebuffer
    SkaShaderInstance* screen_shader_instance = ska_frame_buffer_get_screen_shader();
    ska_shader_use(screen_shader_instance->shader);

    // Apply shader instance params
    ska_renderer_set_shader_instance_params(screen_shader_instance);

    glBindVertexArray(ska_frame_buffer_get_quad_vao());
    glViewport(viewport_data->position.x, viewport_data->position.y, viewport_data->size.w, viewport_data->size.h);


    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, ska_frame_buffer_get_color_buffer_texture());	// use the color attachment texture as the texture of the quad plane
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
#endif
}

#ifdef SKA_RENDER_TO_FRAMEBUFFER
void ska_renderer_process_and_flush_batches_just_framebuffer(const SkaColor *background_color) {
    ska_frame_buffer_bind();

    // Clear framebuffer with background color
    ska_renderer_clear(background_color->r, background_color->g, background_color->b, background_color->a);

    ska_renderer_flush_batches();

    ska_frame_buffer_unbind();
}
#endif

void ska_renderer_print_opengl_errors() {
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
void ska_renderer_set_global_shader_param_time(f32 time_value) {
    global_shader_param_time = time_value;
}

void ska_renderer_set_shader_instance_params(SkaShaderInstance* shader_instance) {
    // Set global shader params first
    ska_shader_set_float(shader_instance->shader, "TIME", global_shader_param_time);

    // Now set shader params specific to the shader instance
    if (shader_instance->params_dirty && shader_instance->param_map->size > 0) {
        SKA_STRING_HASH_MAP_FOR_EACH(shader_instance->param_map, iter) {
            SkaShaderParam* param = (SkaShaderParam*)iter.value;
            switch (param->type) {
                case SkaShaderParamType_BOOL: {
                    ska_shader_set_bool(shader_instance->shader, param->name, param->value.bool_value);
                    break;
                }
                case SkaShaderParamType_INT: {
                    ska_shader_set_int(shader_instance->shader, param->name, param->value.int_value);
                    break;
                }
                case SkaShaderParamType_FLOAT: {
                    ska_shader_set_float(shader_instance->shader, param->name, param->value.float_value);
                    break;
                }
                case SkaShaderParamType_FLOAT2: {
                    ska_shader_set_vec2_float(shader_instance->shader, param->name, param->value.float2_value.x,param->value.float2_value.y);
                    break;
                }
                case SkaShaderParamType_FLOAT3: {
                    ska_shader_set_vec3_float(shader_instance->shader, param->name, param->value.float3_value.x,param->value.float3_value.y, param->value.float3_value.z);
                    break;
                }
                case SkaShaderParamType_FLOAT4: {
                    ska_shader_set_vec4_float(shader_instance->shader, param->name, param->value.float4_value.x,param->value.float4_value.y, param->value.float4_value.z,param->value.float4_value.w);
                    break;
                }
            }
        }
        shader_instance->params_dirty = false;
    }
}

#endif // #if SKA_RENDERING
