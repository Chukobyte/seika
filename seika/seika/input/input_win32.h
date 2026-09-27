#pragma once

#include <seika/defines.h>

#if defined(PLATFORM_WINDOWS) && SKA_INPUT

#include <seika/input/input.h>

SkaInputKey ska_input_win32_vkcode_to_keycode(uint32 vkcode);

#endif // #if defined(PLATFORM_WINDOWS) && SKA_INPUT
