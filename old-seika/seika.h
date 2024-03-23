#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

bool sf_initialize_simple(const char* title, int windowWidth, int windowHeight);
bool sf_initialize(const char* title,
                   int windowWidth,
                   int windowHeight,
                   int resolutionWidth,
                   int resolutionHeight,
                   uint32_t audioWavSampleRate,
                   bool maintainAspectRatio);
void sf_process_inputs();
void sf_fixed_update(float deltaTime);
void sf_render();
bool sf_is_running();
uint32_t sf_get_ticks();
bool sf_set_vsync_enabled(bool enabled);
void sf_delay(uint32_t timeToWait);
void sf_shutdown();

#ifdef __cplusplus
}
#endif
