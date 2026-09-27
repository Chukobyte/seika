#pragma once

#include <seika/defines.h>

#if defined(PLATFORM_MACOS) && SKA_INPUT

#include <seika/input/input.h>

#ifdef __cplusplus
extern "C" {
#endif

// 'mac_virtual_keycode' is an NSEvent.keyCode (kVK_*) -- a fixed hardware-position keycode, not
// a layout-resolved character. Intentional for game input (WASD wants physical position), but
// means non-QWERTY layouts map letter keys by position, not by the character they produce.
SkaInputKey ska_input_cocoa_keycode_to_keycode(unsigned short mac_virtual_keycode);

#ifdef __cplusplus
}
#endif

#endif // #if defined(PLATFORM_MACOS) && SKA_INPUT
