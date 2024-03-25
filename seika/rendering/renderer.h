#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

#include "texture.h"
#include "font.h"
#include "shader/shader_instance.h"
#include "seika/math/math.h"

#define SKA_RENDERER_MAX_Z_INDEX 200

typedef struct SkaRendererTransform2D {
    mat4 model;
} SkaRendererTransform2D;

void ska_renderer_initialize(int32 inWindowWidth, int32 inWindowHeight, int32 inResolutionWidth, int32 inResolutionHeight, bool maintainAspectRatio);
void ska_renderer_finalize();
void ska_renderer_update_window_size(int32 windowWidth, int32 windowHeight);
void ska_renderer_set_sprite_shader_default_params(SkaShader* shader);
void ska_renderer_queue_sprite_draw(SkaTexture* texture, SkaRect2 sourceRect, SkaSize2D destSize, SkaColor color, bool flipH, bool flipV, const SkaTransform2D* transform2D, int32 zIndex, SkaShaderInstance* shaderInstance);
void ska_renderer_queue_sprite_draw2(SkaTexture* texture, SkaRect2 sourceRect, SkaSize2D destSize, SkaColor color, bool flipH, bool flipV, mat4 trsMatrix, int32 zIndex, SkaShaderInstance* shaderInstance);
void ska_renderer_queue_font_draw_call(SkaFont* font, const char* text, f32 x, f32 y, f32 scale, SkaColor color, int32 zIndex);
void ska_renderer_process_and_flush_batches(const SkaColor *backgroundColor);
void ska_renderer_process_and_flush_batches_just_framebuffer(const SkaColor *backgroundColor);

// Shader params
void ska_renderer_set_global_shader_param_time(f32 timeValue);

#ifdef __cplusplus
}
#endif
