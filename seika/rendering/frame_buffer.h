#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

#include "seika/math/math.h"

bool ska_frame_buffer_initialize(int32 inWindowWidth, int32 inWindowHeight, int32 inResolutionWidth, int32 inResolutionHeight);
void ska_frame_buffer_finalize();
void ska_frame_buffer_bind();
void ska_frame_buffer_unbind();
uint32 ska_frame_buffer_get_color_buffer_texture();
uint32 ska_frame_buffer_get_quad_vao();
void ska_frame_buffer_resize_texture(int32 newWidth, int32 newHeight);
void ska_frame_buffer_set_maintain_aspect_ratio(bool shouldMaintainAspectRatio);

struct SkaShaderInstance* ska_frame_buffer_get_screen_shader();
void ska_frame_buffer_set_screen_shader(struct SkaShaderInstance* shaderInstance);
void ska_frame_buffer_reset_to_default_screen_shader();

typedef struct SkaFrameBufferViewportData {
    SkaVector2i position;
    SkaSize2Di size;
} SkaFrameBufferViewportData;

SkaFrameBufferViewportData ska_frame_buffer_generate_viewport_data(int32 windowWidth, int32 windowHeight);
SkaFrameBufferViewportData* ska_frame_buffer_get_cached_viewport_data();

#ifdef __cplusplus
}
#endif
