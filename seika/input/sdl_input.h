#pragma once

#include <SDL3/SDL.h>

#include "input.h"
#include "sdl_gamepad_db.h"

typedef struct SkaSDLGamepadData {
    SDL_Gamepad* gamepad;
    SkaInputDeviceIndex deviceIndex;
} SkaSDLGamepadData;

typedef struct SkaSDLInputEvent {
    SkaInputSourceType sourceType;
    SkaInputTriggerType triggerType;
    SkaInputDeviceIndex deviceIndex;
    SkaInputKey key;
    f32 axisMotionValue;
} SkaSDLInputEvent;

static SkaSDLGamepadData gamepadData[SKA_INPUT_MAX_DEVICES];
static SkaSDLGamepadData gamepadDataByPlayerIndex[SKA_INPUT_MAX_DEVICES];
static uint32 activeGamepads = 0;

SkaInputKey ska_sdl_keycode_to_input_key(SDL_GamepadButton keycode) {
    switch ((int32)keycode) {
        case SDLK_TAB: return SkaInputKey_KEYBOARD_TAB;
        case SDLK_LEFT: return SkaInputKey_KEYBOARD_LEFT;
        case SDLK_RIGHT: return SkaInputKey_KEYBOARD_RIGHT;
        case SDLK_UP: return SkaInputKey_KEYBOARD_UP;
        case SDLK_DOWN: return SkaInputKey_KEYBOARD_DOWN;
        case SDLK_PAGEDOWN: return SkaInputKey_KEYBOARD_PAGE_DOWN;
        case SDLK_PAGEUP: return SkaInputKey_KEYBOARD_PAGE_UP;
        case SDLK_HOME: return SkaInputKey_KEYBOARD_HOME;
        case SDLK_END: return SkaInputKey_KEYBOARD_END;
        case SDLK_INSERT: return SkaInputKey_KEYBOARD_INSERT;
        case SDLK_DELETE: return SkaInputKey_KEYBOARD_DELETE;
        case SDLK_BACKSPACE: return SkaInputKey_KEYBOARD_BACKSPACE;
        case SDLK_SPACE: return SkaInputKey_KEYBOARD_SPACE;
        case SDLK_RETURN: return SkaInputKey_KEYBOARD_RETURN;
        case SDLK_ESCAPE: return SkaInputKey_KEYBOARD_ESCAPE;
        case SDLK_QUOTE: return SkaInputKey_KEYBOARD_QUOTE;
        case SDLK_COMMA: return SkaInputKey_KEYBOARD_COMMA;
        case SDLK_MINUS: return SkaInputKey_KEYBOARD_MINUS;
        case SDLK_PERIOD: return SkaInputKey_KEYBOARD_PERIOD;
        case SDLK_SLASH: return SkaInputKey_KEYBOARD_SLASH;
        case SDLK_SEMICOLON: return SkaInputKey_KEYBOARD_SEMICOLON;
        case SDLK_EQUALS: return SkaInputKey_KEYBOARD_EQUALS;
        case SDLK_LEFTBRACKET: return SkaInputKey_KEYBOARD_LEFT_BRACKET;
        case SDLK_BACKSLASH: return SkaInputKey_KEYBOARD_BACKSLASH;
        case SDLK_RIGHTBRACKET: return SkaInputKey_KEYBOARD_RIGHT_BRACKET;
        case SDLK_BACKQUOTE: return SkaInputKey_KEYBOARD_BACKQUOTE;
        case SDLK_CAPSLOCK: return SkaInputKey_KEYBOARD_CAPS_LOCK;
        case SDLK_SCROLLLOCK: return SkaInputKey_KEYBOARD_SCROLL_LOCK;
        case SDLK_NUMLOCKCLEAR: return SkaInputKey_KEYBOARD_NUM_LOCK_CLEAR;
        case SDLK_PRINTSCREEN: return SkaInputKey_KEYBOARD_PRINT_SCREEN;
        case SDLK_PAUSE: return SkaInputKey_KEYBOARD_PAUSE;
        case SDLK_KP_0: return SkaInputKey_KEYBOARD_KEYPAD_0;
        case SDLK_KP_1: return SkaInputKey_KEYBOARD_KEYPAD_1;
        case SDLK_KP_2: return SkaInputKey_KEYBOARD_KEYPAD_2;
        case SDLK_KP_3: return SkaInputKey_KEYBOARD_KEYPAD_3;
        case SDLK_KP_4: return SkaInputKey_KEYBOARD_KEYPAD_4;
        case SDLK_KP_5: return SkaInputKey_KEYBOARD_KEYPAD_5;
        case SDLK_KP_6: return SkaInputKey_KEYBOARD_KEYPAD_6;
        case SDLK_KP_7: return SkaInputKey_KEYBOARD_KEYPAD_7;
        case SDLK_KP_8: return SkaInputKey_KEYBOARD_KEYPAD_8;
        case SDLK_KP_9: return SkaInputKey_KEYBOARD_KEYPAD_9;
        case SDLK_KP_PERIOD: return SkaInputKey_KEYBOARD_KEYPAD_PERIOD;
        case SDLK_KP_MULTIPLY: return SkaInputKey_KEYBOARD_KEYPAD_MULTIPLY;
        case SDLK_KP_MINUS: return SkaInputKey_KEYBOARD_KEYPAD_MINUS;
        case SDLK_KP_PLUS: return SkaInputKey_KEYBOARD_KEYPAD_PLUS;
        case SDLK_KP_ENTER: return SkaInputKey_KEYBOARD_KEYPAD_ENTER;
        case SDLK_KP_EQUALS: return SkaInputKey_KEYBOARD_KEYPAD_EQUALS;
        case SDLK_LCTRL: return SkaInputKey_KEYBOARD_LEFT_CONTROL;
        case SDLK_LSHIFT: return SkaInputKey_KEYBOARD_LEFT_SHIFT;
        case SDLK_LALT: return SkaInputKey_KEYBOARD_LEFT_ALT;
        case SDLK_LGUI: return SkaInputKey_KEYBOARD_LEFT_GUI;
        case SDLK_RCTRL: return SkaInputKey_KEYBOARD_RIGHT_CONTROL;
        case SDLK_RSHIFT: return SkaInputKey_KEYBOARD_RIGHT_SHIFT;
        case SDLK_RALT: return SkaInputKey_KEYBOARD_RIGHT_ALT;
        case SDLK_RGUI: return SkaInputKey_KEYBOARD_RIGHT_GUI;
        case SDLK_APPLICATION: return SkaInputKey_KEYBOARD_APPLICATION;
        case SDLK_0: return SkaInputKey_KEYBOARD_NUM_0;
        case SDLK_1: return SkaInputKey_KEYBOARD_NUM_1;
        case SDLK_2: return SkaInputKey_KEYBOARD_NUM_2;
        case SDLK_3: return SkaInputKey_KEYBOARD_NUM_3;
        case SDLK_4: return SkaInputKey_KEYBOARD_NUM_4;
        case SDLK_5: return SkaInputKey_KEYBOARD_NUM_5;
        case SDLK_6: return SkaInputKey_KEYBOARD_NUM_6;
        case SDLK_7: return SkaInputKey_KEYBOARD_NUM_7;
        case SDLK_8: return SkaInputKey_KEYBOARD_NUM_8;
        case SDLK_9: return SkaInputKey_KEYBOARD_NUM_9;
        case SDLK_a: return SkaInputKey_KEYBOARD_A;
        case SDLK_b: return SkaInputKey_KEYBOARD_B;
        case SDLK_c: return SkaInputKey_KEYBOARD_C;
        case SDLK_d: return SkaInputKey_KEYBOARD_D;
        case SDLK_e: return SkaInputKey_KEYBOARD_E;
        case SDLK_f: return SkaInputKey_KEYBOARD_F;
        case SDLK_g: return SkaInputKey_KEYBOARD_G;
        case SDLK_h: return SkaInputKey_KEYBOARD_H;
        case SDLK_i: return SkaInputKey_KEYBOARD_I;
        case SDLK_j: return SkaInputKey_KEYBOARD_J;
        case SDLK_k: return SkaInputKey_KEYBOARD_K;
        case SDLK_l: return SkaInputKey_KEYBOARD_L;
        case SDLK_m: return SkaInputKey_KEYBOARD_M;
        case SDLK_n: return SkaInputKey_KEYBOARD_N;
        case SDLK_o: return SkaInputKey_KEYBOARD_O;
        case SDLK_p: return SkaInputKey_KEYBOARD_P;
        case SDLK_q: return SkaInputKey_KEYBOARD_Q;
        case SDLK_r: return SkaInputKey_KEYBOARD_R;
        case SDLK_s: return SkaInputKey_KEYBOARD_S;
        case SDLK_t: return SkaInputKey_KEYBOARD_T;
        case SDLK_u: return SkaInputKey_KEYBOARD_U;
        case SDLK_v: return SkaInputKey_KEYBOARD_V;
        case SDLK_w: return SkaInputKey_KEYBOARD_W;
        case SDLK_x: return SkaInputKey_KEYBOARD_X;
        case SDLK_y: return SkaInputKey_KEYBOARD_Y;
        case SDLK_z: return SkaInputKey_KEYBOARD_Z;
        case SDLK_F1: return SkaInputKey_KEYBOARD_F1;
        case SDLK_F2: return SkaInputKey_KEYBOARD_F2;
        case SDLK_F3: return SkaInputKey_KEYBOARD_F3;
        case SDLK_F4: return SkaInputKey_KEYBOARD_F4;
        case SDLK_F5: return SkaInputKey_KEYBOARD_F5;
        case SDLK_F6: return SkaInputKey_KEYBOARD_F6;
        case SDLK_F7: return SkaInputKey_KEYBOARD_F7;
        case SDLK_F8: return SkaInputKey_KEYBOARD_F8;
        case SDLK_F9: return SkaInputKey_KEYBOARD_F9;
        case SDLK_F10: return SkaInputKey_KEYBOARD_F10;
        case SDLK_F11: return SkaInputKey_KEYBOARD_F11;
        case SDLK_F12: return SkaInputKey_KEYBOARD_F12;
        case SDLK_F13: return SkaInputKey_KEYBOARD_F13;
        case SDLK_F14: return SkaInputKey_KEYBOARD_F14;
        case SDLK_F15: return SkaInputKey_KEYBOARD_F15;
        case SDLK_F16: return SkaInputKey_KEYBOARD_F16;
        case SDLK_F17: return SkaInputKey_KEYBOARD_F17;
        case SDLK_F18: return SkaInputKey_KEYBOARD_F18;
        case SDLK_F19: return SkaInputKey_KEYBOARD_F19;
        case SDLK_F20: return SkaInputKey_KEYBOARD_F20;
        case SDLK_F21: return SkaInputKey_KEYBOARD_F21;
        case SDLK_F22: return SkaInputKey_KEYBOARD_F22;
        case SDLK_F23: return SkaInputKey_KEYBOARD_F23;
        case SDLK_F24: return SkaInputKey_KEYBOARD_F24;
        case SDLK_AC_BACK: return SkaInputKey_KEYBOARD_APP_BACK;
        case SDLK_AC_FORWARD: return SkaInputKey_KEYBOARD_APP_FORWARD;
        default: return SkaInputKey_INVALID;
    }
}


SkaInputKey ska_sdl_mouse_button_to_input_key(SDL_MouseButtonEvent button) {
    switch (button.button) {
        case SDL_BUTTON_LEFT: return SkaInputKey_MOUSE_BUTTON_LEFT;
        case SDL_BUTTON_RIGHT: return SkaInputKey_MOUSE_BUTTON_RIGHT;
        case SDL_BUTTON_MIDDLE: return SkaInputKey_MOUSE_BUTTON_MIDDLE;
        default: return SkaInputKey_INVALID;
    }
}

SkaInputKey ska_sdl_gamepad_button_to_input_key(SDL_GamepadButton button) {
    switch (button) {
        case SDL_GAMEPAD_BUTTON_SOUTH: return SkaInputKey_GAMEPAD_FACE_BUTTON_SOUTH;
        case SDL_GAMEPAD_BUTTON_EAST: return SkaInputKey_GAMEPAD_FACE_BUTTON_EAST;
        case SDL_GAMEPAD_BUTTON_WEST: return SkaInputKey_GAMEPAD_FACE_BUTTON_WEST;
        case SDL_GAMEPAD_BUTTON_NORTH: return SkaInputKey_GAMEPAD_FACE_BUTTON_NORTH;
        case SDL_GAMEPAD_BUTTON_BACK: return SkaInputKey_GAMEPAD_BACK;
        case SDL_GAMEPAD_BUTTON_START: return SkaInputKey_GAMEPAD_START;
        case SDL_GAMEPAD_BUTTON_LEFT_STICK: return SkaInputKey_GAMEPAD_LEFT_ANALOG_BUTTON;
        case SDL_GAMEPAD_BUTTON_RIGHT_STICK: return SkaInputKey_GAMEPAD_RIGHT_ANALOG_BUTTON;
        case SDL_GAMEPAD_BUTTON_LEFT_SHOULDER: return SkaInputKey_GAMEPAD_LEFT_SHOULDER;
        case SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER: return SkaInputKey_GAMEPAD_RIGHT_SHOULDER;
        case SDL_GAMEPAD_BUTTON_DPAD_UP: return SkaInputKey_GAMEPAD_DPAD_UP;
        case SDL_GAMEPAD_BUTTON_DPAD_DOWN: return SkaInputKey_GAMEPAD_DPAD_DOWN;
        case SDL_GAMEPAD_BUTTON_DPAD_LEFT: return SkaInputKey_GAMEPAD_DPAD_LEFT;
        case SDL_GAMEPAD_BUTTON_DPAD_RIGHT: return SkaInputKey_GAMEPAD_DPAD_RIGHT;
        default: return SkaInputKey_INVALID;
    }
}

SkaInputKey ska_sdl_gamepad_axis_to_input_key(SDL_GamepadAxis axis) {
    switch (axis) {
        case SDL_GAMEPAD_AXIS_LEFTX: return SkaInputKey_GAMEPAD_LEFT_ANALOG_2D_AXIS_X;
        case SDL_GAMEPAD_AXIS_LEFTY: return SkaInputKey_GAMEPAD_LEFT_ANALOG_2D_AXIS_Y;
        case SDL_GAMEPAD_AXIS_RIGHTX: return SkaInputKey_GAMEPAD_RIGHT_ANALOG_2D_AXIS_X;
        case SDL_GAMEPAD_AXIS_RIGHTY: return SkaInputKey_GAMEPAD_RIGHT_ANALOG_2D_AXIS_Y;
        case SDL_GAMEPAD_AXIS_LEFT_TRIGGER: return SkaInputKey_GAMEPAD_LEFT_TRIGGER;
        case SDL_GAMEPAD_AXIS_RIGHT_TRIGGER: return SkaInputKey_GAMEPAD_RIGHT_TRIGGER;
        default: return SkaInputKey_INVALID;
    }
}

bool ska_sdl_load_gamepad_mappings() {
    const int loadResult = SDL_AddGamepadMapping(SKA_GAMEPAD_DB_STR);
    return loadResult >= 0;
}

static inline void ska_sdl_notify_input_event(const SkaSDLInputEvent* inputEvent) {
    ska_input_register_input_event(inputEvent->sourceType, inputEvent->key, inputEvent->triggerType, inputEvent->deviceIndex, inputEvent->axisMotionValue);
}

void ska_sdl_process_event(SDL_Event event) {
    SkaSDLInputEvent inputEvent = {
        .sourceType = SkaInputSourceType_INVALID,
        .triggerType = SkaInputTriggerType_INVALID,
        .deviceIndex = SKA_INPUT_FIRST_PLAYER_DEVICE_INDEX,
        .key = SkaInputKey_INVALID,
        .axisMotionValue = 0.0f
    };

    switch (event.type) {
        // Mouse
        case SDL_EVENT_MOUSE_MOTION: {
            SkaMouse* mouse = ska_input_get_mouse();
            mouse->position = (SkaVector2){ .x = (f32)event.motion.x, .y = (f32)event.motion.y };
            break;
        }

        case SDL_EVENT_MOUSE_BUTTON_DOWN:
        case SDL_EVENT_MOUSE_BUTTON_UP: {
            inputEvent.sourceType = SkaInputSourceType_MOUSE;
            inputEvent.triggerType = event.type == SDL_EVENT_MOUSE_BUTTON_DOWN ? SkaInputTriggerType_PRESSED : SkaInputTriggerType_RELEASED;
            inputEvent.key = ska_sdl_mouse_button_to_input_key(event.button);
            break;
        }
        // Keyboard
        case SDL_EVENT_KEY_DOWN:
        case SDL_EVENT_KEY_UP: {
            inputEvent.sourceType = SkaInputSourceType_KEYBOARD;
            inputEvent.triggerType = event.type == SDL_EVENT_KEY_DOWN ? SkaInputTriggerType_PRESSED : SkaInputTriggerType_RELEASED,
            inputEvent.key = ska_sdl_keycode_to_input_key(event.key.keysym.sym);
            break;
        }
        // Gamepad
        case SDL_EVENT_GAMEPAD_ADDED: {
            const SDL_JoystickID deviceIndex = event.gdevice.which;
            SDL_Gamepad* newGamepad = SDL_OpenGamepad(deviceIndex);
            const int playerIndex = SDL_GetGamepadPlayerIndex(newGamepad);
            const SkaSDLGamepadData newGamepadData = { .gamepad = newGamepad, .deviceIndex = playerIndex };
            gamepadData[deviceIndex] = newGamepadData;
            gamepadDataByPlayerIndex[playerIndex] = newGamepadData;
            activeGamepads++;

            inputEvent.sourceType = SkaInputSourceType_GAMEPAD;
            inputEvent.triggerType = SkaInputTriggerType_DEVICE_ADDED;
            inputEvent.deviceIndex = playerIndex;
            break;
        }
        case SDL_EVENT_GAMEPAD_REMOVED: {
            const SDL_JoystickID deviceIndex = event.gdevice.which;
            SDL_Gamepad* removedGamepad = gamepadData[deviceIndex].gamepad;
            const SkaInputDeviceIndex playerIndex = gamepadData[deviceIndex].deviceIndex;
            SDL_CloseGamepad(removedGamepad);
            const SkaSDLGamepadData defaultGamepadData = (SkaSDLGamepadData){ .gamepad = NULL, .deviceIndex = -1 };
            gamepadData[deviceIndex] = defaultGamepadData;
            gamepadDataByPlayerIndex[playerIndex] = defaultGamepadData;
            activeGamepads--;

            inputEvent.sourceType = SkaInputSourceType_GAMEPAD;
            inputEvent.triggerType = SkaInputTriggerType_DEVICE_REMOVED;
            inputEvent.deviceIndex = playerIndex;
            break;
        }
        case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
        case SDL_EVENT_GAMEPAD_BUTTON_UP: {
            const SDL_JoystickID deviceIndex = event.jbutton.which;
            const SkaInputDeviceIndex playerIndex = gamepadData[deviceIndex].deviceIndex;
            const bool isButtonPressed = event.jbutton.state == SDL_PRESSED;
            const SDL_GamepadButton buttonValue = event.jbutton.button;

            inputEvent.sourceType = SkaInputSourceType_GAMEPAD;
            inputEvent.triggerType = isButtonPressed ? SkaInputTriggerType_PRESSED : SkaInputTriggerType_RELEASED;
            inputEvent.deviceIndex = playerIndex;
            inputEvent.key = ska_sdl_gamepad_button_to_input_key(buttonValue);
            break;
        }
        default:
            break;
    }
    ska_sdl_notify_input_event(&inputEvent);
}

// Call after looping through all sdl input events for a frame
void ska_sdl_process_axis_events() {
#define SKA_SDL_GAMEPAD_AXIS_DEAD_ZONE 8000
#define SKA_MAX_AXIS_VALUES 6

    static const SDL_GamepadAxis AXIS_KEYS[SKA_MAX_AXIS_VALUES] = {
        SDL_GAMEPAD_AXIS_LEFTX, SDL_GAMEPAD_AXIS_LEFTY,
        SDL_GAMEPAD_AXIS_RIGHTX, SDL_GAMEPAD_AXIS_RIGHTY,
        SDL_GAMEPAD_AXIS_LEFT_TRIGGER, SDL_GAMEPAD_AXIS_RIGHT_TRIGGER
    };
    static bool hasGamepadStoppedAxisMotion[SKA_INPUT_MAX_DEVICES][SKA_MAX_AXIS_VALUES] = {
        {true, true, true, true, true, true},
        {true, true, true, true, true, true},
        {true, true, true, true, true, true},
        {true, true, true, true, true, true},
        {true, true, true, true, true, true},
        {true, true, true, true, true, true},
        {true, true, true, true, true, true},
        {true, true, true, true, true, true}
    };

    for (uint32 i = 0; i < activeGamepads; i++) {
        SkaSDLGamepadData* currentGamepadData = &gamepadDataByPlayerIndex[i];
        for (SDL_GamepadAxis axis = 0; axis < SKA_MAX_AXIS_VALUES; axis++) {
            const int16 axisValue = SDL_GetGamepadAxis(currentGamepadData->gamepad, axis);
            const SkaInputTriggerType triggerType =
                    axisValue < SKA_SDL_GAMEPAD_AXIS_DEAD_ZONE && axisValue > -SKA_SDL_GAMEPAD_AXIS_DEAD_ZONE
                    ? SkaInputTriggerType_AXIS_STOPPED_MOTION : SkaInputTriggerType_AXIS_IN_MOTION;
            // Skip sending event if axis motion is already stopped
            bool *hasStoppedAxisMotion = &hasGamepadStoppedAxisMotion[i][axis];
            if (triggerType == SkaInputTriggerType_AXIS_STOPPED_MOTION) {
                if (*hasStoppedAxisMotion) {
                    continue;
                } else {
                    *hasStoppedAxisMotion = true;
                }
            } else {
                *hasStoppedAxisMotion = false;
            }
            const f32 axisValueNormalized = ska_math_map_to_range((f32) axisValue, (f32) INT16_MIN, (f32) INT16_MAX,-1.0f, 1.0f);
            const SkaSDLInputEvent inputEvent = {
                .sourceType = SkaInputSourceType_GAMEPAD,
                .triggerType = triggerType,
                .deviceIndex = (SkaInputDeviceIndex)i,
                .key = ska_sdl_gamepad_axis_to_input_key(axis),
                .axisMotionValue = axisValueNormalized
            };
            ska_sdl_notify_input_event(&inputEvent);
        }
    }

#undef SKA_MAX_AXIS_VALUES
}
