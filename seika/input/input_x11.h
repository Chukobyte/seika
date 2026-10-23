#pragma once

#include <seika/defines.h>

#if defined(PLATFORM_LINUX) && SKA_INPUT

#include <X11/Xlib.h>

#include <seika/input/input.h>

SkaInputKey ska_input_x11_keysym_to_keycode(Display* display, unsigned int hardware_keycode);

#endif // #if defined(PLATFORM_LINUX) && SKA_INPUT
