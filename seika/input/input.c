#if SKA_INPUT

#include "input.h"
#include <math.h>
#include "seika/string.h"
#include "seika/assert.h"
#include "seika/memory.h"

#define SKA_INPUT_MAX_KEY_STATE_CLEANUP_SIZE 16

typedef struct SkaAxisInputValues {
    SkaVector2 left;
    SkaVector2 right;
} SkaAxisInputValues;

typedef struct SkaInputKeyState {
    bool is_pressed;
    bool is_just_pressed;
    bool is_just_released;
    f32 strength;
    SkaAxisInputValues prev_axis_values[SKA_INPUT_MAX_DEVICES];
} SkaInputKeyState;

typedef struct SkaInputStateCleanup {
    SkaInputKey key;
    SkaInputDeviceIndex device_index;
} SkaInputStateCleanup;

typedef struct SkaInputActionData {
    SkaInputAction  action;
    SkaInputActionHandle handle;
} SkaInputActionData;

typedef struct SkaInputState {
    SkaInputKeyState input_key_state[SKA_INPUT_MAX_DEVICES][SkaInputKey_NUMBER_OF_KEYS];

    SkaAxisInputValues prev_axis_inputs[SKA_INPUT_MAX_DEVICES];

    SkaInputStateCleanup cleanup_key_state_just_pressed[SKA_INPUT_MAX_KEY_STATE_CLEANUP_SIZE];
    SkaInputStateCleanup cleanup_key_state_just_released[SKA_INPUT_MAX_KEY_STATE_CLEANUP_SIZE];
    usize cleanup_key_state_just_pressed_count;
    usize cleanup_key_state_just_released_count;

    SkaInputActionHandle input_action_handle_index;
    SkaInputActionData input_action_data[SKA_INPUT_MAX_DEVICES][SKA_INPUT_MAX_INPUT_ACTIONS];
} SkaInputState;

#define DEFAULT_INPUT_STATE {0}

static SkaInputState input_state = DEFAULT_INPUT_STATE;
static SkaMouse global_mouse = {0};
static bool is_input_active = false;

static SkaInputKey get_axis_direction_key(SkaInputKey axis2_d_key, f32 axis_value);
static f32* get_prev_input_axis_value(SkaInputKey key, SkaAxisInputValues* axis_input_values);
static void update_axis_key_state(SkaInputKey key, SkaInputInteractionStatus interaction_status, f32 axis_value, SkaInputDeviceIndex device_index);

bool ska_input_initialize() {
    SKA_ASSERT(!is_input_active);

    is_input_active = true;
    return true;
}

void ska_input_finalize() {
    SKA_ASSERT(is_input_active);

    is_input_active = false;
}

const char* ska_input_key_to_string(SkaInputKey key) {
    static const char* key_names[SkaInputKey_NUMBER_OF_KEYS] = {
        [SkaInputKey_INVALID] = "Invalid",
        // Gamepad
        [SkaInputKey_GAMEPAD_DPAD_DOWN] = "Gamepad Dpad Down",
        [SkaInputKey_GAMEPAD_DPAD_UP] = "Gamepad Dpad Up",
        [SkaInputKey_GAMEPAD_DPAD_LEFT] = "Gamepad Dpad Left",
        [SkaInputKey_GAMEPAD_DPAD_RIGHT] = "Gamepad Dpad Right",
        [SkaInputKey_GAMEPAD_FACE_BUTTON_NORTH] = "Gamepad Face Button North",
        [SkaInputKey_GAMEPAD_FACE_BUTTON_SOUTH] = "Gamepad Face Button South",
        [SkaInputKey_GAMEPAD_FACE_BUTTON_EAST] = "Gamepad Face Button East",
        [SkaInputKey_GAMEPAD_FACE_BUTTON_WEST] = "Gamepad Face Button West",
        [SkaInputKey_GAMEPAD_START] = "Gamepad Start",
        [SkaInputKey_GAMEPAD_BACK] = "Gamepad Back",
        [SkaInputKey_GAMEPAD_LEFT_SHOULDER] = "Gamepad Left Shoulder",
        [SkaInputKey_GAMEPAD_LEFT_TRIGGER] = "Gamepad Left Trigger",
        [SkaInputKey_GAMEPAD_LEFT_ANALOG_BUTTON] = "Gamepad Left Analog Button",
        [SkaInputKey_GAMEPAD_LEFT_ANALOG_2D_AXIS_X] = "Gamepad Left Analog 2D Axis X",
        [SkaInputKey_GAMEPAD_LEFT_ANALOG_2D_AXIS_Y] = "Gamepad Left Analog 2D Axis Y",
        [SkaInputKey_GAMEPAD_RIGHT_SHOULDER] = "Gamepad Right Shoulder",
        [SkaInputKey_GAMEPAD_RIGHT_TRIGGER] = "Gamepad Right Trigger",
        [SkaInputKey_GAMEPAD_RIGHT_ANALOG_BUTTON] = "Gamepad Right Analog Button",
        [SkaInputKey_GAMEPAD_RIGHT_ANALOG_2D_AXIS_X] = "Gamepad Right Analog 2D Axis X",
        [SkaInputKey_GAMEPAD_RIGHT_ANALOG_2D_AXIS_Y] = "Gamepad Right Analog 2D Axis Y",
        [SkaInputKey_GAMEPAD_GUIDE] = "Gamepad Guide",
        [SkaInputKey_GAMEPAD_PADDLE1] = "Gamepad Paddle 1",
        [SkaInputKey_GAMEPAD_PADDLE2] = "Gamepad Paddle 2",
        [SkaInputKey_GAMEPAD_PADDLE3] = "Gamepad Paddle 3",
        [SkaInputKey_GAMEPAD_PADDLE4] = "Gamepad Paddle 4",
        [SkaInputKey_GAMEPAD_MISC1] = "Gamepad Misc 1",
        [SkaInputKey_GAMEPAD_MISC2] = "Gamepad Misc 2",
        [SkaInputKey_GAMEPAD_TOUCHPAD] = "Gamepad Touchpad",
        [SkaInputKey_GAMEPAD_LEFT_ANALOG_LEFT] = "Gamepad Left Analog Left",
        [SkaInputKey_GAMEPAD_LEFT_ANALOG_RIGHT] = "Gamepad Left Analog Right",
        [SkaInputKey_GAMEPAD_LEFT_ANALOG_UP] = "Gamepad Left Analog Up",
        [SkaInputKey_GAMEPAD_LEFT_ANALOG_DOWN] = "Gamepad Left Analog Down",
        [SkaInputKey_GAMEPAD_RIGHT_ANALOG_LEFT] = "Gamepad Right Analog Left",
        [SkaInputKey_GAMEPAD_RIGHT_ANALOG_RIGHT] = "Gamepad Right Analog Right",
        [SkaInputKey_GAMEPAD_RIGHT_ANALOG_UP] = "Gamepad Right Analog Up",
        [SkaInputKey_GAMEPAD_RIGHT_ANALOG_DOWN] = "Gamepad Right Analog Down",
        // Keyboard
        [SkaInputKey_KEYBOARD_TAB] = "Tab",
        [SkaInputKey_KEYBOARD_LEFT] = "Left",
        [SkaInputKey_KEYBOARD_RIGHT] = "Right",
        [SkaInputKey_KEYBOARD_UP] = "Up",
        [SkaInputKey_KEYBOARD_DOWN] = "Down",
        [SkaInputKey_KEYBOARD_PAGE_DOWN] = "Page Down",
        [SkaInputKey_KEYBOARD_PAGE_UP] = "Page Up",
        [SkaInputKey_KEYBOARD_HOME] = "Home",
        [SkaInputKey_KEYBOARD_END] = "End",
        [SkaInputKey_KEYBOARD_INSERT] = "Insert",
        [SkaInputKey_KEYBOARD_DELETE] = "Delete",
        [SkaInputKey_KEYBOARD_BACKSPACE] = "Backspace",
        [SkaInputKey_KEYBOARD_SPACE] = "Space",
        [SkaInputKey_KEYBOARD_RETURN] = "Return",
        [SkaInputKey_KEYBOARD_ESCAPE] = "Escape",
        [SkaInputKey_KEYBOARD_QUOTE] = "Quote",
        [SkaInputKey_KEYBOARD_COMMA] = "Comma",
        [SkaInputKey_KEYBOARD_MINUS] = "Minus",
        [SkaInputKey_KEYBOARD_PERIOD] = "Period",
        [SkaInputKey_KEYBOARD_SLASH] = "Slash",
        [SkaInputKey_KEYBOARD_SEMICOLON] = "Semicolon",
        [SkaInputKey_KEYBOARD_EQUALS] = "Equals",
        [SkaInputKey_KEYBOARD_LEFT_BRACKET] = "Left Bracket",
        [SkaInputKey_KEYBOARD_RIGHT_BRACKET] = "Right Bracket",
        [SkaInputKey_KEYBOARD_BACKSLASH] = "Backslash",
        [SkaInputKey_KEYBOARD_BACKQUOTE] = "Backquote",
        [SkaInputKey_KEYBOARD_CAPS_LOCK] = "Caps Lock",
        [SkaInputKey_KEYBOARD_SCROLL_LOCK] = "Scroll Lock",
        [SkaInputKey_KEYBOARD_NUM_LOCK_CLEAR] = "Num Lock Clear",
        [SkaInputKey_KEYBOARD_PRINT_SCREEN] = "Print Screen",
        [SkaInputKey_KEYBOARD_PAUSE] = "Pause",
        [SkaInputKey_KEYBOARD_KEYPAD_0] = "Keypad 0",
        [SkaInputKey_KEYBOARD_KEYPAD_1] = "Keypad 1",
        [SkaInputKey_KEYBOARD_KEYPAD_2] = "Keypad 2",
        [SkaInputKey_KEYBOARD_KEYPAD_3] = "Keypad 3",
        [SkaInputKey_KEYBOARD_KEYPAD_4] = "Keypad 4",
        [SkaInputKey_KEYBOARD_KEYPAD_5] = "Keypad 5",
        [SkaInputKey_KEYBOARD_KEYPAD_6] = "Keypad 6",
        [SkaInputKey_KEYBOARD_KEYPAD_7] = "Keypad 7",
        [SkaInputKey_KEYBOARD_KEYPAD_8] = "Keypad 8",
        [SkaInputKey_KEYBOARD_KEYPAD_9] = "Keypad 9",
        [SkaInputKey_KEYBOARD_KEYPAD_PERIOD] = "Keypad Period",
        [SkaInputKey_KEYBOARD_KEYPAD_DIVIDE] = "Keypad Divide",
        [SkaInputKey_KEYBOARD_KEYPAD_MULTIPLY] = "Keypad Multiply",
        [SkaInputKey_KEYBOARD_KEYPAD_MINUS] = "Keypad Minus",
        [SkaInputKey_KEYBOARD_KEYPAD_PLUS] = "Keypad Plus",
        [SkaInputKey_KEYBOARD_KEYPAD_ENTER] = "Keypad Enter",
        [SkaInputKey_KEYBOARD_KEYPAD_EQUALS] = "Keypad Equals",
        [SkaInputKey_KEYBOARD_LEFT_CONTROL] = "Left Control",
        [SkaInputKey_KEYBOARD_LEFT_SHIFT] = "Left Shift",
        [SkaInputKey_KEYBOARD_LEFT_ALT] = "Left Alt",
        [SkaInputKey_KEYBOARD_LEFT_GUI] = "Left GUI",
        [SkaInputKey_KEYBOARD_RIGHT_CONTROL] = "Right Control",
        [SkaInputKey_KEYBOARD_RIGHT_SHIFT] = "Right Shift",
        [SkaInputKey_KEYBOARD_RIGHT_ALT] = "Right Alt",
        [SkaInputKey_KEYBOARD_RIGHT_GUI] = "Right GUI",
        [SkaInputKey_KEYBOARD_APPLICATION] = "Application",
        [SkaInputKey_KEYBOARD_NUM_0] = "0",
        [SkaInputKey_KEYBOARD_NUM_1] = "1",
        [SkaInputKey_KEYBOARD_NUM_2] = "2",
        [SkaInputKey_KEYBOARD_NUM_3] = "3",
        [SkaInputKey_KEYBOARD_NUM_4] = "4",
        [SkaInputKey_KEYBOARD_NUM_5] = "5",
        [SkaInputKey_KEYBOARD_NUM_6] = "6",
        [SkaInputKey_KEYBOARD_NUM_7] = "7",
        [SkaInputKey_KEYBOARD_NUM_8] = "8",
        [SkaInputKey_KEYBOARD_NUM_9] = "9",
        [SkaInputKey_KEYBOARD_A] = "A",
        [SkaInputKey_KEYBOARD_B] = "B",
        [SkaInputKey_KEYBOARD_C] = "C",
        [SkaInputKey_KEYBOARD_D] = "D",
        [SkaInputKey_KEYBOARD_E] = "E",
        [SkaInputKey_KEYBOARD_F] = "F",
        [SkaInputKey_KEYBOARD_G] = "G",
        [SkaInputKey_KEYBOARD_H] = "H",
        [SkaInputKey_KEYBOARD_I] = "I",
        [SkaInputKey_KEYBOARD_J] = "J",
        [SkaInputKey_KEYBOARD_K] = "K",
        [SkaInputKey_KEYBOARD_L] = "L",
        [SkaInputKey_KEYBOARD_M] = "M",
        [SkaInputKey_KEYBOARD_N] = "N",
        [SkaInputKey_KEYBOARD_O] = "O",
        [SkaInputKey_KEYBOARD_P] = "P",
        [SkaInputKey_KEYBOARD_Q] = "Q",
        [SkaInputKey_KEYBOARD_R] = "R",
        [SkaInputKey_KEYBOARD_S] = "S",
        [SkaInputKey_KEYBOARD_T] = "T",
        [SkaInputKey_KEYBOARD_U] = "U",
        [SkaInputKey_KEYBOARD_V] = "V",
        [SkaInputKey_KEYBOARD_W] = "W",
        [SkaInputKey_KEYBOARD_X] = "X",
        [SkaInputKey_KEYBOARD_Y] = "Y",
        [SkaInputKey_KEYBOARD_Z] = "Z",
        [SkaInputKey_KEYBOARD_F1] = "F1",
        [SkaInputKey_KEYBOARD_F2] = "F2",
        [SkaInputKey_KEYBOARD_F3] = "F3",
        [SkaInputKey_KEYBOARD_F4] = "F4",
        [SkaInputKey_KEYBOARD_F5] = "F5",
        [SkaInputKey_KEYBOARD_F6] = "F6",
        [SkaInputKey_KEYBOARD_F7] = "F7",
        [SkaInputKey_KEYBOARD_F8] = "F8",
        [SkaInputKey_KEYBOARD_F9] = "F9",
        [SkaInputKey_KEYBOARD_F10] = "F10",
        [SkaInputKey_KEYBOARD_F11] = "F11",
        [SkaInputKey_KEYBOARD_F12] = "F12",
        [SkaInputKey_KEYBOARD_F13] = "F13",
        [SkaInputKey_KEYBOARD_F14] = "F14",
        [SkaInputKey_KEYBOARD_F15] = "F15",
        [SkaInputKey_KEYBOARD_F16] = "F16",
        [SkaInputKey_KEYBOARD_F17] = "F17",
        [SkaInputKey_KEYBOARD_F18] = "F18",
        [SkaInputKey_KEYBOARD_F19] = "F19",
        [SkaInputKey_KEYBOARD_F20] = "F20",
        [SkaInputKey_KEYBOARD_F21] = "F21",
        [SkaInputKey_KEYBOARD_F22] = "F22",
        [SkaInputKey_KEYBOARD_F23] = "F23",
        [SkaInputKey_KEYBOARD_F24] = "F24",
        [SkaInputKey_KEYBOARD_APP_FORWARD] = "App Forward",
        [SkaInputKey_KEYBOARD_APP_BACK] = "App Back",
        // Mouse
        [SkaInputKey_MOUSE_BUTTON_LEFT] = "Mouse Button Left",
        [SkaInputKey_MOUSE_BUTTON_RIGHT] = "Mouse Button Right",
        [SkaInputKey_MOUSE_BUTTON_MIDDLE] = "Mouse Button Middle",
    };
    return key_names[key];
}

SkaInputKey ska_input_string_to_key(const char* key_name) {
    for (SkaInputKey key = 0; key < SkaInputKey_NUMBER_OF_KEYS; ++key) {
        if (strcmp(key_name, ska_input_key_to_string(key)) == 0) {
            return key;
        }
    }
    return SkaInputKey_INVALID; // Return invalid if not found
}

bool ska_input_is_keyboard_key(SkaInputKey key) {
    return key >= SKA_INPUT_KEY_KEYBOARD_ENUM_START && key <= SKA_INPUT_KEY_KEYBOARD_ENUM_END;
}

bool ska_input_is_mouse_key(SkaInputKey key) {
    return key >= SKA_INPUT_KEY_MOUSE_ENUM_START && key <= SKA_INPUT_KEY_MOUSE_ENUM_END;
}

bool ska_input_is_gamepad_key(SkaInputKey key) {
    return key >= SKA_INPUT_KEY_GAMEPAD_ENUM_START && key <= SKA_INPUT_KEY_GAMEPAD_ENUM_END;
}

// Events
void ska_input_register_gamepad_added_event(SkaInputDeviceIndex device_index) {
    // Ensures a newly connected device starts from clean state instead of possibly inheriting
    // a stale value left behind by whatever previously occupied this device_index slot.
    ska_input_reset_gamepad(device_index);
}

void ska_input_register_gamepad_removed_event(SkaInputDeviceIndex device_index) {
    ska_input_reset_gamepad(device_index);
}

void ska_input_register_mouse_move_event(SkaVector2i new_position) {
    global_mouse.position = new_position;
}

void ska_input_register_mouse_scroll_event(SkaVector2 delta) {
    global_mouse.scroll.x += delta.x;
    global_mouse.scroll.y += delta.y;
}

void ska_input_register_input_event3(SkaInputSourceType source_type, SkaInputKey key, SkaInputTriggerType trigger_type) {
    ska_input_register_input_event2(source_type, key, trigger_type, SKA_INPUT_FIRST_PLAYER_DEVICE_INDEX);
}

void ska_input_register_input_event2(SkaInputSourceType source_type, SkaInputKey key, SkaInputTriggerType trigger_type, SkaInputDeviceIndex device_index) {
    ska_input_register_input_event(source_type, key, trigger_type, device_index, 0.0f);
}

void ska_input_register_input_event(SkaInputSourceType source_type, SkaInputKey key, SkaInputTriggerType trigger_type, SkaInputDeviceIndex device_index, f32 gamepad_axis_value) {
    SkaInputInteractionStatus interaction_status = SkaInputInteractionStatus_PRESSED;
    SkaInputKeyState* key_state = &input_state.input_key_state[device_index][key];

    switch (trigger_type) {
        case SkaInputTriggerType_PRESSED: {
            if (!key_state->is_pressed) {
                // interactionStatus = SkaInputInteractionStatus_JUST_PRESSED;
                key_state->is_just_pressed = true;
                SKA_ASSERT(input_state.cleanup_key_state_just_pressed_count + 1 <= SKA_INPUT_MAX_KEY_STATE_CLEANUP_SIZE);
                input_state.cleanup_key_state_just_pressed
                    [input_state.cleanup_key_state_just_pressed_count++] =
                    (SkaInputStateCleanup){.key = key,
                                           .device_index = device_index};
            } else {
                key_state->is_just_pressed = false;
            }
            key_state->is_pressed = true;
            key_state->is_just_released = false;
            key_state->strength = 1.0f;
            break;
        }
        case SkaInputTriggerType_RELEASED: {
            if (key_state->is_just_released) {
                return;
            }
            key_state->is_pressed = false;
            key_state->is_just_pressed = false;
            key_state->is_just_released = true;
            key_state->strength = 0.0f;
            // interactionStatus = SkaInputInteractionStatus_JUST_RELEASED;
            SKA_ASSERT(input_state.cleanup_key_state_just_released_count + 1 <= SKA_INPUT_MAX_KEY_STATE_CLEANUP_SIZE);
            input_state.cleanup_key_state_just_released
                [input_state.cleanup_key_state_just_released_count++] =
                (SkaInputStateCleanup){.key = key,
                                       .device_index = device_index};
            break;
        }
        case SkaInputTriggerType_AXIS_IN_MOTION: {
            if (!key_state->is_pressed) {
                interaction_status = SkaInputInteractionStatus_AXIS_STARTED_MOTION;
                key_state->is_just_pressed = true;
                SKA_ASSERT(input_state.cleanup_key_state_just_pressed_count + 1 <= SKA_INPUT_MAX_KEY_STATE_CLEANUP_SIZE);
                input_state.cleanup_key_state_just_pressed
                    [input_state.cleanup_key_state_just_pressed_count++] =
                    (SkaInputStateCleanup){.key = key,
                                           .device_index = device_index};
            } else {
                interaction_status = SkaInputInteractionStatus_AXIS_IN_MOTION;
                key_state->is_just_pressed = false;
            }
            key_state->is_pressed = true;
            key_state->is_just_released = false;
            key_state->strength = gamepad_axis_value;

            if (key != SkaInputKey_GAMEPAD_LEFT_TRIGGER && key != SkaInputKey_GAMEPAD_RIGHT_TRIGGER) {
                update_axis_key_state(key, interaction_status, gamepad_axis_value, device_index);
            }
            break;
        }
        case SkaInputTriggerType_AXIS_STOPPED_MOTION: {
            if (key_state->is_just_released) {
                return;
            }
            interaction_status = SkaInputInteractionStatus_AXIS_STOPPED_MOTION;
            key_state->is_pressed = false;
            key_state->is_just_pressed = false;
            key_state->is_just_released = true;
            key_state->strength = 0.0f;
            SKA_ASSERT(input_state.cleanup_key_state_just_released_count + 1 <= SKA_INPUT_MAX_KEY_STATE_CLEANUP_SIZE);
            input_state.cleanup_key_state_just_released
                [input_state.cleanup_key_state_just_released_count++] =
                (SkaInputStateCleanup){.key = key,
                                       .device_index = device_index};

            if (key != SkaInputKey_GAMEPAD_LEFT_TRIGGER && key != SkaInputKey_GAMEPAD_RIGHT_TRIGGER) {
                update_axis_key_state(key, interaction_status, gamepad_axis_value, device_index);
            }
            break;
        }
        case SkaInputTriggerType_DEVICE_ADDED:
        case SkaInputTriggerType_DEVICE_REMOVED:
        case SkaInputTriggerType_INVALID:
            break;
    }

//    const SkaInputEvent inputEvent = (SkaInputEvent) { .sourceType = sourceType, .interactionStatus = interactionStatus, .key = key, .deviceIndex = deviceIndex, gamepadAxisValue = gamepadAxisValue };
}

bool ska_input_is_key_pressed(SkaInputKey key, SkaInputDeviceIndex device_index) {
    return input_state.input_key_state[device_index][key].is_pressed;
}
bool ska_input_is_key_just_pressed(SkaInputKey key, SkaInputDeviceIndex device_index) {
    return input_state.input_key_state[device_index][key].is_just_pressed;
}
bool ska_input_is_key_just_released(SkaInputKey key, SkaInputDeviceIndex device_index) {
    return input_state.input_key_state[device_index][key].is_just_released;
}
f32 ska_input_get_key_strength(SkaInputKey key, SkaInputDeviceIndex device_index) {
    return input_state.input_key_state[device_index][key].strength;
}

SkaVector2 ska_input_get_axis_input(SkaInputAxis axis, SkaInputDeviceIndex device_index) {
    struct SkaAxis2DInputKeys {
        SkaInputKey x;
        SkaInputKey y;
    };

    const struct SkaAxis2DInputKeys axis_keys = axis == SkaInputAxis_LEFT
        ? (struct SkaAxis2DInputKeys) { .x = SkaInputKey_GAMEPAD_LEFT_ANALOG_2D_AXIS_X, .y = SkaInputKey_GAMEPAD_LEFT_ANALOG_2D_AXIS_Y }
        : (struct SkaAxis2DInputKeys) { .x = SkaInputKey_GAMEPAD_RIGHT_ANALOG_2D_AXIS_X, .y = SkaInputKey_GAMEPAD_RIGHT_ANALOG_2D_AXIS_Y };
    const SkaInputKeyState* x_key_state = &input_state.input_key_state[device_index][axis_keys.x];
    const SkaInputKeyState* y_key_state = &input_state.input_key_state[device_index][axis_keys.y];
    return (SkaVector2) { .x = x_key_state->strength, .y = y_key_state->strength };
}

SkaMouse* ska_input_get_mouse() {
    return &global_mouse;
}

// Input Action
SkaInputActionHandle ska_input_add_input_action(const char* action_name, const SkaInputActionValue* action_values, SkaInputDeviceIndex device_index) {
    SkaInputActionHandle action_handle = ska_input_find_input_action_handle(action_name, device_index);
    if (action_handle == SKA_INPUT_INVALID_INPUT_ACTION_HANDLE) {
        SKA_ASSERT(input_state.input_action_handle_index + 1 < SKA_INPUT_MAX_INPUT_ACTIONS);
        action_handle = input_state.input_action_handle_index++;
        SkaInputActionData* action_data = &input_state.input_action_data[device_index][action_handle];
        action_data->handle = action_handle;
        action_data->action.name = ska_strdup(action_name);
        action_data->action.action_values_count = 0;
    }

    SkaInputActionData* action_data = &input_state.input_action_data[device_index][action_handle];
    for (usize i = 0; i < SKA_INPUT_MAX_INPUT_ACTION_VALUES; i++) {
        if (!action_values[i].key) {
            break;
        }
        SKA_ASSERT(action_data->action.action_values_count + 1 < SKA_INPUT_MAX_INPUT_ACTION_VALUES);
        action_data->action.action_values[action_data->action.action_values_count++] = action_values[i];
    }
    return action_handle;
}

SkaInputAction* ska_input_get_input_action(SkaInputActionHandle handle, SkaInputDeviceIndex device_index) {
    SkaInputActionData* action_data = &input_state.input_action_data[device_index][handle];
    if (action_data->handle) {
        return &action_data->action;
    }
    return NULL;
}

bool ska_input_remove_input_action(SkaInputActionHandle handle, SkaInputDeviceIndex device_index) {
    SkaInputActionData* action_data = &input_state.input_action_data[device_index][handle];
    if (action_data->handle != SKA_INPUT_INVALID_INPUT_ACTION_HANDLE) {
        SKA_FREE(action_data->action.name);
        action_data->action.name = NULL;
        action_data->handle = SKA_INPUT_INVALID_INPUT_ACTION_HANDLE;
        return true;
    }
    return false;
}

SkaInputActionHandle ska_input_find_input_action_handle(const char* action_name, SkaInputDeviceIndex device_index) {
    for (SkaInputActionHandle handle = 0; handle < SKA_INPUT_MAX_INPUT_ACTIONS; handle++) {
        SkaInputActionData* action_data = &input_state.input_action_data[device_index][handle];
        // TODO: Shouldn't have to check for NULL name as handle should be enough
        if (action_data->handle == SKA_INPUT_INVALID_INPUT_ACTION_HANDLE || action_data->action.name == NULL) {
            continue;
        }
        if (strcmp(action_data->action.name, action_name) == 0) {
            return action_data->handle;
        }
    }
    return SKA_INPUT_INVALID_INPUT_ACTION_HANDLE;
}

bool ska_input_is_input_action_pressed(SkaInputActionHandle handle, SkaInputDeviceIndex device_index) {
    const SkaInputActionData* action_data = &input_state.input_action_data[device_index][handle];
    for (usize i = 0; i < action_data->action.action_values_count; i++) {
        const SkaInputActionValue* action_value = &action_data->action.action_values[i];
        if (ska_input_is_key_pressed(action_value->key, device_index)) {
            return true;
        }
    }
    return false;
}

bool ska_input_is_input_action_just_pressed(SkaInputActionHandle handle, SkaInputDeviceIndex device_index) {
    const SkaInputActionData* action_data = &input_state.input_action_data[device_index][handle];
    for (usize i = 0; i < action_data->action.action_values_count; i++) {
        const SkaInputActionValue* action_value = &action_data->action.action_values[i];
        if (ska_input_is_key_just_pressed(action_value->key, device_index)) {
            return true;
        }
    }
    return false;
}

bool ska_input_is_input_action_just_released(SkaInputActionHandle handle, SkaInputDeviceIndex device_index) {
    const SkaInputActionData* action_data = &input_state.input_action_data[device_index][handle];
    for (usize i = 0; i < action_data->action.action_values_count; i++) {
        const SkaInputActionValue* action_value = &action_data->action.action_values[i];
        if (ska_input_is_key_just_released(action_value->key, device_index)) {
            return true;
        }
    }
    return false;
}

f32 ska_input_get_input_action_strength(SkaInputActionHandle handle, SkaInputDeviceIndex device_index) {
    const SkaInputActionData* action_data = &input_state.input_action_data[device_index][handle];
    for (usize i = 0; i < action_data->action.action_values_count; i++) {
        const SkaInputActionValue* action_value = &action_data->action.action_values[i];
        const f32 key_strength = ska_input_get_key_strength(action_value->key, device_index);
        if (key_strength >= action_value->strength_threshold) {
            return key_strength;
        }
    }
    return 0.0f;
}

void ska_input_new_frame() {
    for (usize i = 0; i < input_state.cleanup_key_state_just_pressed_count; i++) {
        const SkaInputStateCleanup* state_cleanup = &input_state.cleanup_key_state_just_pressed[i];
        SkaInputKeyState* key_state = &input_state.input_key_state[state_cleanup->device_index][state_cleanup->key];
        key_state->is_just_pressed = false;
    }
    for (usize i = 0; i < input_state.cleanup_key_state_just_released_count; i++) {
        const SkaInputStateCleanup* state_cleanup = &input_state.cleanup_key_state_just_released[i];
        SkaInputKeyState* key_state = &input_state.input_key_state[state_cleanup->device_index][state_cleanup->key];
        key_state->is_just_released = false;
    }
    input_state.cleanup_key_state_just_pressed_count = 0;
    input_state.cleanup_key_state_just_released_count = 0;
    global_mouse.scroll = (SkaVector2){ 0.0f, 0.0f };
}

void ska_input_reset_gamepad(SkaInputDeviceIndex device_index) {
    SKA_INPUT_KEY_GAMEPAD_FOR_EACH(key) {
        input_state.input_key_state[device_index][key] = (SkaInputKeyState){0};
    }
}

void ska_input_reset(SkaInputDeviceIndex device_index) {
    input_state = (SkaInputState)DEFAULT_INPUT_STATE;
}

// Private
SkaInputKey get_axis_direction_key(SkaInputKey axis2_d_key, f32 axis_value) {
    switch (axis2_d_key) {
        case SkaInputKey_GAMEPAD_LEFT_ANALOG_2D_AXIS_X:
            return axis_value > 0 ? SkaInputKey_GAMEPAD_LEFT_ANALOG_RIGHT : SkaInputKey_GAMEPAD_LEFT_ANALOG_LEFT;
        case SkaInputKey_GAMEPAD_LEFT_ANALOG_2D_AXIS_Y:
            return axis_value > 0 ? SkaInputKey_GAMEPAD_LEFT_ANALOG_DOWN : SkaInputKey_GAMEPAD_LEFT_ANALOG_UP;
        case SkaInputKey_GAMEPAD_RIGHT_ANALOG_2D_AXIS_X:
            return axis_value > 0 ? SkaInputKey_GAMEPAD_RIGHT_ANALOG_RIGHT : SkaInputKey_GAMEPAD_RIGHT_ANALOG_LEFT;
        case SkaInputKey_GAMEPAD_RIGHT_ANALOG_2D_AXIS_Y:
            return axis_value > 0 ? SkaInputKey_GAMEPAD_RIGHT_ANALOG_DOWN : SkaInputKey_GAMEPAD_RIGHT_ANALOG_UP;
        default: {
            SKA_ASSERT_FMT(false, "Not a valid axis input key!");
            return SkaInputKey_INVALID;
        }
    }
}

f32* get_prev_input_axis_value(SkaInputKey key, SkaAxisInputValues* axis_input_values) {
    switch (key) {
        case SkaInputKey_GAMEPAD_LEFT_ANALOG_2D_AXIS_X: return &axis_input_values->left.x;
        case SkaInputKey_GAMEPAD_LEFT_ANALOG_2D_AXIS_Y: return &axis_input_values->left.y;
        case SkaInputKey_GAMEPAD_RIGHT_ANALOG_2D_AXIS_X: return &axis_input_values->right.x;
        case SkaInputKey_GAMEPAD_RIGHT_ANALOG_2D_AXIS_Y: return &axis_input_values->right.y;
        default: {
            SKA_ASSERT_FMT(false, "Not a valid axis input key!");
            return &axis_input_values->left.x;
        }
    }
}

static void press_axis_direction_key(SkaInputKey direction_key, f32 strength, SkaInputDeviceIndex device_index) {
    SkaInputKeyState* key_state = &input_state.input_key_state[device_index][direction_key];
    if (!key_state->is_pressed) {
        key_state->is_just_pressed = true;
        SKA_ASSERT(input_state.cleanup_key_state_just_pressed_count + 1 <= SKA_INPUT_MAX_KEY_STATE_CLEANUP_SIZE);
        input_state.cleanup_key_state_just_pressed[input_state.cleanup_key_state_just_pressed_count++] =
            (SkaInputStateCleanup){ .key = direction_key, .device_index = device_index };
    }
    key_state->is_pressed = true;
    key_state->is_just_released = false;
    key_state->strength = strength;
}

static void release_axis_direction_key(SkaInputKey direction_key, SkaInputDeviceIndex device_index) {
    SkaInputKeyState* key_state = &input_state.input_key_state[device_index][direction_key];
    if (!key_state->is_pressed) {
        return;
    }
    key_state->is_pressed = false;
    key_state->is_just_pressed = false;
    key_state->is_just_released = true;
    key_state->strength = 0.0f;
    SKA_ASSERT(input_state.cleanup_key_state_just_released_count + 1 <= SKA_INPUT_MAX_KEY_STATE_CLEANUP_SIZE);
    input_state.cleanup_key_state_just_released[input_state.cleanup_key_state_just_released_count++] =
        (SkaInputStateCleanup){ .key = direction_key, .device_index = device_index };
}

void update_axis_key_state(SkaInputKey key, SkaInputInteractionStatus interaction_status, f32 axis_value, SkaInputDeviceIndex device_index) {
    f32* prev_value = get_prev_input_axis_value(key, &input_state.prev_axis_inputs[device_index]);
    const SkaInputKey prev_direction_key = *prev_value != 0.0f ? get_axis_direction_key(key, *prev_value) : SkaInputKey_INVALID;
    switch (interaction_status) {
        case SkaInputInteractionStatus_AXIS_STARTED_MOTION:
        case SkaInputInteractionStatus_AXIS_IN_MOTION: {
            const SkaInputKey direction_key = get_axis_direction_key(key, axis_value);
            // Stick crossed center within one poll (never hit the deadzone) -- release the old direction
            if (prev_direction_key != SkaInputKey_INVALID && prev_direction_key != direction_key) {
                release_axis_direction_key(prev_direction_key, device_index);
            }
            press_axis_direction_key(direction_key, fabsf(axis_value), device_index);
            *prev_value = axis_value;
            break;
        }
        case SkaInputInteractionStatus_AXIS_STOPPED_MOTION: {
            if (prev_direction_key != SkaInputKey_INVALID) {
                release_axis_direction_key(prev_direction_key, device_index);
            }
            *prev_value = 0.0f;
            break;
        }
        default: {
            SKA_ASSERT_FMT(false, "Invalid interaction status fo 2d axis key when trying to update key state!");
        }
    }
}

#endif // if SKA_INPUT
