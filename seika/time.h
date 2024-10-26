#pragma once

#if SKA_INPUT || SKA_RENDERING

#include "seika/defines.h"

uint32 ska_get_ticks();
void ska_delay(uint32 milliseconds);

#endif // #if SKA_INPUT || SKA_RENDERING
