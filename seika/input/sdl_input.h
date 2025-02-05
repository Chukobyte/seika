#pragma once

#if SKA_INPUT && SKA_SDL

#include <SDL3/SDL.h>

#include "input.h"

typedef struct SkaSDLInputEvent {
    SkaInputSourceType sourceType;
    SkaInputTriggerType triggerType;
    SkaInputDeviceIndex deviceIndex;
    SkaInputKey key;
    f32 axisMotionValue;
} SkaSDLInputEvent;

SkaInputKey ska_sdl_keycode_to_input_key(SDL_GamepadButton keycode);
SkaInputKey ska_sdl_mouse_button_to_input_key(SDL_MouseButtonEvent button);
SkaInputKey ska_sdl_gamepad_button_to_input_key(SDL_GamepadButton button);
SkaInputKey ska_sdl_gamepad_axis_to_input_key(SDL_GamepadAxis axis);
bool ska_sdl_load_gamepad_mappings();
void ska_sdl_notify_input_event(const SkaSDLInputEvent* inputEvent);
void ska_sdl_process_event(SDL_Event event);
// Call after looping through all sdl input events for a frame
void ska_sdl_process_axis_events();

#endif // if SKA_INPUT && SKA_SDL
