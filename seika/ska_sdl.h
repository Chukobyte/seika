#pragma once

#include "seika/defines.h"

#if SKA_SDL && (SKA_RENDINER || SKA_INPUT)
bool ska_sdl_update();
#endif // #if SKA_SDL && (SKA_RENDINER || SKA_INPUT)