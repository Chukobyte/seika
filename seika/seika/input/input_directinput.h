#pragma once

#include <seika/defines.h>

#if defined(PLATFORM_WINDOWS) && SKA_INPUT

#include <windows.h>

// Initializes the shared IDirectInput8 COM object. Must be called once before any of the
// other ska_input_directinput_* functions (from ska_win_init, alongside WNDCLASS registration).
bool ska_input_directinput_init(HINSTANCE h_instance);

// Enumerates DI8DEVCLASS_GAMECTRL devices, tracking additions/removals against the previously-
// known set, and resolves new ones against the gamepad_db by GUID, firing added/removed events.
// Called once from ska_win_create_window() (needs a live HWND), then periodically from
// ska_input_directinput_poll_gamepads() to catch hot-plug changes.
void ska_input_directinput_enumerate(HWND hwnd);

// Polls all connected devices and translates changes into ska_input_register_input_event*()
// calls via each device's gamepad_db mapping. Call once per frame.
void ska_input_directinput_poll_gamepads(void);

// Releases all tracked devices and the shared IDirectInput8 object.
void ska_input_directinput_shutdown(void);

#endif // #if defined(PLATFORM_WINDOWS) && SKA_INPUT
