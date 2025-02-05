#pragma once

#if SKA_SDL

#ifdef __cplusplus
extern "C" {
#endif

#include "seika/defines.h"

uint32 ska_get_ticks();
void ska_delay(uint32 milliseconds);

#ifdef __cplusplus
}
#endif

#endif // #if SKA_SDL
