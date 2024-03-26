#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "defines.h"

bool ska_init();
void ska_shutdown();
bool ska_init_all(const char* title, int32 windowWidth, int32 windowHeight, int32 resolutionWidth, int32 resolutionHeight);
bool ska_init_all2(const char* title, int32 windowWidth, int32 windowHeight, int32 resolutionWidth, int32 resolutionHeight, uint32 audioWavSampleRate, bool maintainAspectRatio);
void ska_shutdown_all();
void ska_update();
void ska_fixed_update(f32 deltaTime);
bool ska_is_running();
uint32 ska_get_ticks();
void ska_delay(uint32 timeToWait);
bool ska_set_vsync_enabled(bool enabled);
bool ska_print_errors();

bool ska_window_init(const char* title, int32 windowWidth, int32 windowHeight);
bool ska_window_init2(const char* title, int32 windowWidth, int32 windowHeight, int32 resolutionWidth, int32 resolutionHeight, bool maintainAspectRatio);
void ska_window_shutdown();
void ska_window_render();

bool ska_input_init();
void ska_input_shutdown();

bool ska_audio_init();
bool ska_audio_init2(uint32 audioWavSampleRate);
void ska_audio_shutdown();

#ifdef __cplusplus
}
#endif