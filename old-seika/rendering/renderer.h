#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

#include "texture.h"
#include "font.h"
#include "shader/shader_instance.h"
#include "../math/se_math.h"

#define SE_RENDERER_MAX_Z_INDEX 200

typedef struct SKARendererTransform2D {
    mat4 model;
} SKARendererTransform2D;

void se_renderer_initialize(int inWindowWidth, int inWindowHeight, int inResolutionWidth, int inResolutionHeight, bool maintainAspectRatio);
void se_renderer_finalize();
void se_renderer_update_window_size(int windowWidth, int windowHeight);
void se_renderer_set_sprite_shader_default_params(SEShader* shader);
void ska_renderer_queue_sprite_draw(SETexture* texture, SKARect2 sourceRect, SKASize2D destSize, SKAColor color, bool flipH, bool flipV, const SKATransform2D* transform2D, int zIndex, SEShaderInstance* shaderInstance);
void ska_renderer_queue_sprite_draw2(SETexture* texture, SKARect2 sourceRect, SKASize2D destSize, SKAColor color, bool flipH, bool flipV, mat4 trsMatrix, int zIndex, SEShaderInstance* shaderInstance);
void se_renderer_queue_font_draw_call(SEFont* font, const char* text, float x, float y, float scale, SKAColor color, int zIndex);
void se_renderer_process_and_flush_batches(const SKAColor* backgroundColor);
void se_renderer_process_and_flush_batches_just_framebuffer(const SKAColor* backgroundColor);

// Shader params
void se_renderer_set_global_shader_param_time(float timeValue);

#ifdef __cplusplus
}
#endif
