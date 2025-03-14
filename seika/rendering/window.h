#pragma once

#if SKA_RENDERING && SKA_SDL

#ifdef __cplusplus
extern "C" {
#endif

#include "seika/defines.h"

struct SkaColor;

typedef struct SkaWindowProperties {
    const char* title;
    int32 windowWidth;
    int32 windowHeight;
    int32 resolutionWidth;
    int32 resolutionHeight;
    bool maintainAspectRatio;
} SkaWindowProperties;

bool ska_window_initialize(SkaWindowProperties props);
void ska_window_finalize();
void ska_window_render(const struct SkaColor* backgroundColor);
bool ska_window_set_vsync(bool enabled);

#ifdef __cplusplus
}
#endif

#endif // #if SKA_RENDERING && SKA_SDL

