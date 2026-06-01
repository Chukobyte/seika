#if SKA_RENDERING

#include "renderer.h"

#include <stdlib.h>

#include "render_context.h"
#include "shader/shader.h"
#include "shader/shader_cache.h"
#include "shader/shader_source.h"
#include "seika/window/window.h"
#include "seika/assert.h"
#include "../logger.h"
#include "seika/data_structures/static_array.h"

// TODO: Fix render to framebuffer for 3D
#define SKA_RENDER_TO_FRAMEBUFFER

#ifdef SKA_RENDER_TO_FRAMEBUFFER
#include "frame_buffer.h"
#endif

// Global Shader Params
static f32 globalShaderParamTime = 0.0f;

// Renderer
void ska_renderer_init(SkaRendererInitProps props) {
    // glEnable(GL_CULL_FACE); // TODO: Used in 2D, we can flip state for 2D and 3D as needed...
    glEnable(GL_BLEND);
    glDisable(GL_MULTISAMPLE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    ska_render_context_initialize((SkaSize2Di){ props.resolutionWidth, props.resolutionHeight });
    ska_renderer_update_window_size(props.windowWidth, props.windowHeight);
    ska_shader_cache_initialize();
    if (props.init2D) {
        ska_renderer_init_2d();
    }
    if (props.initProps3D) {
        ska_renderer_init_3d((SkaRenderer3DInitProps) { .fov = props.initProps3D->fov, .nearPlane = props.initProps3D->nearPlane, .farPlane = props.initProps3D->farPlane });
    }
#ifdef SKA_RENDER_TO_FRAMEBUFFER
    // Initialize framebuffer
    SKA_ASSERT_FMT(ska_frame_buffer_initialize(props.windowWidth, props.windowHeight, props.resolutionWidth, props.resolutionHeight), "Framebuffer didn't initialize!");
    ska_frame_buffer_set_maintain_aspect_ratio(props.maintainAspectRatio);
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

void ska_renderer_update_window_size(int32 windowWidth, int32 windowHeight) {
#ifdef SKA_RENDER_TO_FRAMEBUFFER
    const SkaFrameBufferViewportData data = ska_frame_buffer_generate_viewport_data(windowWidth, windowHeight);
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
}

void ska_renderer_process_and_flush_batches(const SkaColor* backgroundColor) {
#ifdef SKA_RENDER_TO_FRAMEBUFFER
    ska_frame_buffer_bind();
#endif

    // Clear framebuffer with background color
    ska_renderer_clear(backgroundColor->r, backgroundColor->g, backgroundColor->b, backgroundColor->a);

#ifdef SKA_RENDER_TO_FRAMEBUFFER
    SkaFrameBufferViewportData* viewportData = ska_frame_buffer_get_cached_viewport_data();
    glViewport(0, 0, viewportData->size.w, viewportData->size.h);
#endif

    ska_renderer_flush_batches();

#ifdef SKA_RENDER_TO_FRAMEBUFFER
    ska_frame_buffer_unbind();

    // Clear screen texture background
    static const SkaColor screenBackgroundColor = {0.0f, 0.0f, 0.0f, 1.0f };
    ska_renderer_clear(screenBackgroundColor.r, screenBackgroundColor.g, screenBackgroundColor.b, screenBackgroundColor.a);
    // Draw screen texture from framebuffer
    SkaShaderInstance* screenShaderInstance = ska_frame_buffer_get_screen_shader();
    ska_shader_use(screenShaderInstance->shader);

    // Apply shader instance params
    ska_renderer_set_shader_instance_params(screenShaderInstance);

    glBindVertexArray(ska_frame_buffer_get_quad_vao());
    glViewport(viewportData->position.x, viewportData->position.y, viewportData->size.w, viewportData->size.h);


    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, ska_frame_buffer_get_color_buffer_texture());	// use the color attachment texture as the texture of the quad plane
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
#endif
}

#ifdef SKA_RENDER_TO_FRAMEBUFFER
void ska_renderer_process_and_flush_batches_just_framebuffer(const SkaColor *backgroundColor) {
    ska_frame_buffer_bind();

    // Clear framebuffer with background color
    ska_renderer_clear(backgroundColor->r, backgroundColor->g, backgroundColor->b, backgroundColor->a);

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
void ska_renderer_set_global_shader_param_time(f32 timeValue) {
    globalShaderParamTime = timeValue;
}

void ska_renderer_set_shader_instance_params(SkaShaderInstance* shaderInstance) {
    // Set global shader params first
    ska_shader_set_float(shaderInstance->shader, "TIME", globalShaderParamTime);

    // Now set shader params specific to the shader instance
    if (shaderInstance->paramsDirty && shaderInstance->paramMap->size > 0) {
        SKA_STRING_HASH_MAP_FOR_EACH(shaderInstance->paramMap, iter) {
            SkaStringHashMapNode* node = iter.pair;
            SkaShaderParam* param = (SkaShaderParam*)node->value;
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

#endif // #if SKA_RENDERING
