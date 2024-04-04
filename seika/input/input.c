#include "input.h"
#include "seika/assert.h"
#include "seika/memory.h"

#define SKA_INPUT_MAX_KEY_STATE_CLEANUP_SIZE 16

typedef struct SkaAxisInputValues {
    SkaVector2 left;
    SkaVector2 right;
} SkaAxisInputValues;

typedef struct SkaInputKeyState {
    bool isPressed;
    bool isJustPressed;
    bool isJustReleased;
    f32 strength;
    SkaAxisInputValues prevAxisValues[SKA_INPUT_MAX_DEVICES];
} SkaInputKeyState;

typedef struct SkaInputStateCleanup {
    SkaInputKey key;
    SkaInputDeviceIndex deviceIndex;
} SkaInputStateCleanup;

typedef struct SkaInputActionData {
    SkaInputAction  action;
    SkaInputActionHandle handle;
} SkaInputActionData;

typedef struct SkaInputState {
    SkaInputKeyState inputKeyState[SKA_INPUT_MAX_DEVICES][SkaInputKey_NUMBER_OF_KEYS];

    SkaAxisInputValues prevAxisInputs[SKA_INPUT_MAX_DEVICES];

    SkaInputStateCleanup cleanupKeyStateJustPressed[SKA_INPUT_MAX_KEY_STATE_CLEANUP_SIZE];
    SkaInputStateCleanup cleanupKeyStateJustReleased[SKA_INPUT_MAX_KEY_STATE_CLEANUP_SIZE];
    usize cleanupKeyStateJustPressedCount;
    usize cleanupKeyStateJustReleasedCount;

    SkaInputActionHandle inputActionHandleIndex;
    SkaInputActionData inputActionData[SKA_INPUT_MAX_DEVICES][SKA_INPUT_MAX_INPUT_ACTIONS];
} SkaInputState;

#define DEFAULT_INPUT_STATE {0}

static SkaInputState inputState = DEFAULT_INPUT_STATE;
static SkaMouse globalMouse = {0};

static SkaInputKey get_key_from_2d_axis_key(SkaInputKey axis2DKey, SkaAxisInputValues* axisInputValues, f32 axisValue);
static void set_prev_input_axis_value(SkaInputKey key, SkaAxisInputValues* axisInputValues, f32 axisValue);
static void update_axis_key_state(SkaInputKey key, SkaInputInteractionStatus interactionStatus, f32 axisValue, SkaInputDeviceIndex deviceIndex);

const char* ska_input_key_to_string(SkaInputKey key) {
    static const char* keyNames[SkaInputKey_NUMBER_OF_KEYS] = {
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
    return keyNames[key];
}

SkaInputKey ska_input_string_to_key(const char* keyName) {
    for (SkaInputKey key = 0; key < SkaInputKey_NUMBER_OF_KEYS; ++key) {
        if (strcmp(keyName, ska_input_key_to_string(key)) == 0) {
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
void ska_input_register_gamepad_added_event(SkaInputDeviceIndex deviceIndex) {}

void ska_input_register_gamepad_removed_event(SkaInputDeviceIndex deviceIndex) {
    ska_input_reset_gamepad(deviceIndex);
}

void ska_input_register_input_event3(SkaInputSourceType sourceType, SkaInputKey key, SkaInputTriggerType triggerType) {
    ska_input_register_input_event2(sourceType, key, triggerType, SKA_INPUT_FIRST_PLAYER_DEVICE_INDEX);
}

void ska_input_register_input_event2(SkaInputSourceType sourceType, SkaInputKey key, SkaInputTriggerType triggerType, SkaInputDeviceIndex deviceIndex) {
    ska_input_register_input_event(sourceType, key, triggerType, deviceIndex, 0.0f);
}

void ska_input_register_input_event(SkaInputSourceType sourceType, SkaInputKey key, SkaInputTriggerType triggerType, SkaInputDeviceIndex deviceIndex, f32 gamepadAxisValue) {
    SkaInputInteractionStatus interactionStatus = SkaInputInteractionStatus_PRESSED;
    SkaInputKeyState* keyState = &inputState.inputKeyState[deviceIndex][key];

    switch (triggerType) {
        case SkaInputTriggerType_PRESSED: {
            if (!keyState->isPressed) {
                interactionStatus = SkaInputInteractionStatus_JUST_PRESSED;
                keyState->isJustPressed = true;
                SKA_ASSERT(inputState.cleanupKeyStateJustPressedCount + 1 <= SKA_INPUT_MAX_KEY_STATE_CLEANUP_SIZE);
                inputState.cleanupKeyStateJustPressed[inputState.cleanupKeyStateJustPressedCount++] = (SkaInputStateCleanup) { .key = key, .deviceIndex = deviceIndex };
            } else {
                keyState->isJustPressed = false;
            }
            keyState->isPressed = true;
            keyState->isJustReleased = false;
            keyState->strength = 1.0f;
            break;
        }
        case SkaInputTriggerType_RELEASED: {
            if (keyState->isJustReleased) {
                return;
            }
            keyState->isPressed = false;
            keyState->isJustPressed = false;
            keyState->isJustReleased = true;
            keyState->strength = 0.0f;
            interactionStatus = SkaInputInteractionStatus_JUST_RELEASED;
            SKA_ASSERT(inputState.cleanupKeyStateJustReleasedCount + 1 <= SKA_INPUT_MAX_KEY_STATE_CLEANUP_SIZE);
            inputState.cleanupKeyStateJustReleased[inputState.cleanupKeyStateJustReleasedCount++] = (SkaInputStateCleanup) { .key = key, .deviceIndex = deviceIndex };
            break;
        }
        case SkaInputTriggerType_AXIS_IN_MOTION: {
            if (!keyState->isPressed) {
                interactionStatus = SkaInputInteractionStatus_AXIS_STARTED_MOTION;
                keyState->isJustPressed = true;
                SKA_ASSERT(inputState.cleanupKeyStateJustPressedCount + 1 <= SKA_INPUT_MAX_KEY_STATE_CLEANUP_SIZE);
                inputState.cleanupKeyStateJustPressed[inputState.cleanupKeyStateJustPressedCount++] = (SkaInputStateCleanup) { .key = key, .deviceIndex = deviceIndex };
            } else {
                interactionStatus = SkaInputInteractionStatus_AXIS_IN_MOTION;
                keyState->isJustPressed = false;
            }
            keyState->isPressed = true;
            keyState->isJustReleased = false;
            keyState->strength = gamepadAxisValue;

            if (key != SkaInputKey_GAMEPAD_LEFT_TRIGGER && key != SkaInputKey_GAMEPAD_RIGHT_TRIGGER) {
                update_axis_key_state(key, interactionStatus, gamepadAxisValue, deviceIndex);
            }
            break;
        }
        case SkaInputTriggerType_AXIS_STOPPED_MOTION: {
            if (keyState->isJustReleased) {
                return;
            }
            interactionStatus = SkaInputInteractionStatus_AXIS_STOPPED_MOTION;
            keyState->isPressed = false;
            keyState->isJustPressed = false;
            keyState->isJustReleased = true;
            keyState->strength = 0.0f;
            SKA_ASSERT(inputState.cleanupKeyStateJustReleasedCount + 1 <= SKA_INPUT_MAX_KEY_STATE_CLEANUP_SIZE);
            inputState.cleanupKeyStateJustReleased[inputState.cleanupKeyStateJustReleasedCount++] = (SkaInputStateCleanup) { .key = key, .deviceIndex = deviceIndex };

            if (key != SkaInputKey_GAMEPAD_LEFT_TRIGGER && key != SkaInputKey_GAMEPAD_RIGHT_TRIGGER) {
                update_axis_key_state(key, interactionStatus, gamepadAxisValue, deviceIndex);
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

bool ska_input_is_key_pressed(SkaInputKey key, SkaInputDeviceIndex deviceIndex) {
    return inputState.inputKeyState[deviceIndex][key].isPressed;
}
bool ska_input_is_key_just_pressed(SkaInputKey key, SkaInputDeviceIndex deviceIndex) {
    return inputState.inputKeyState[deviceIndex][key].isJustPressed;
}
bool ska_input_is_key_just_released(SkaInputKey key, SkaInputDeviceIndex deviceIndex) {
    return inputState.inputKeyState[deviceIndex][key].isJustReleased;
}
f32 ska_input_get_key_strength(SkaInputKey key, SkaInputDeviceIndex deviceIndex) {
    return inputState.inputKeyState[deviceIndex][key].strength;
}

SkaVector2 ska_input_get_axis_input(SkaInputAxis axis, SkaInputDeviceIndex deviceIndex) {
    struct SkaAxis2DInputKeys {
        SkaInputKey x;
        SkaInputKey y;
    };

    const struct SkaAxis2DInputKeys axisKeys = axis == SkaInputAxis_LEFT
        ? (struct SkaAxis2DInputKeys) { .x = SkaInputKey_GAMEPAD_LEFT_ANALOG_2D_AXIS_X, .y = SkaInputKey_GAMEPAD_LEFT_ANALOG_2D_AXIS_Y }
        : (struct SkaAxis2DInputKeys) { .x = SkaInputKey_GAMEPAD_RIGHT_ANALOG_2D_AXIS_X, .y = SkaInputKey_GAMEPAD_RIGHT_ANALOG_2D_AXIS_Y };
    const SkaInputKeyState* xKeyState = &inputState.inputKeyState[deviceIndex][axisKeys.x];
    const SkaInputKeyState* yKeyState = &inputState.inputKeyState[deviceIndex][axisKeys.y];
    return (SkaVector2) { .x = xKeyState->strength, .y = yKeyState->strength };
}

SkaMouse* ska_input_get_mouse() {
    return &globalMouse;
}

// Input Action
SkaInputActionHandle ska_input_add_input_action(const char* actionName, const SkaInputActionValue* actionValues, SkaInputDeviceIndex deviceIndex) {
    SkaInputActionHandle actionHandle = ska_input_find_input_action_handle(actionName, deviceIndex);
    if (actionHandle == SKA_INPUT_INVALID_INPUT_ACTION_HANDLE) {
        SKA_ASSERT(inputState.inputActionHandleIndex + 1 < SKA_INPUT_MAX_INPUT_ACTIONS);
        actionHandle = inputState.inputActionHandleIndex++;
        SkaInputActionData* actionData = &inputState.inputActionData[deviceIndex][actionHandle];
        actionData->handle = actionHandle;
        actionData->action.name = ska_strdup(actionName);
        actionData->action.actionValuesCount = 0;
    }

    SkaInputActionData* actionData = &inputState.inputActionData[deviceIndex][actionHandle];
    for (usize i = 0; i < SKA_INPUT_MAX_INPUT_ACTION_VALUES; i++) {
        if (!actionValues[i].key) {
            break;
        }
        SKA_ASSERT(actionData->action.actionValuesCount + 1 < SKA_INPUT_MAX_INPUT_ACTION_VALUES);
        actionData->action.actionValues[actionData->action.actionValuesCount++] = actionValues[i];
    }
    return actionHandle;
}

SkaInputAction* ska_input_get_input_action(SkaInputActionHandle handle, SkaInputDeviceIndex deviceIndex) {
    SkaInputActionData* actionData = &inputState.inputActionData[deviceIndex][handle];
    if (actionData->handle) {
        return &actionData->action;
    }
    return NULL;
}

bool ska_input_remove_input_action(SkaInputActionHandle handle, SkaInputDeviceIndex deviceIndex) {
    SkaInputActionData* actionData = &inputState.inputActionData[deviceIndex][handle];
    if (actionData->handle != SKA_INPUT_INVALID_INPUT_ACTION_HANDLE) {
        SKA_MEM_FREE(actionData->action.name);
        actionData->action.name = NULL;
        actionData->handle = SKA_INPUT_INVALID_INPUT_ACTION_HANDLE;
        return true;
    }
    return false;
}

SkaInputActionHandle ska_input_find_input_action_handle(const char* actionName, SkaInputDeviceIndex deviceIndex) {
    for (SkaInputActionHandle handle = 0; handle < SKA_INPUT_MAX_INPUT_ACTIONS; handle++) {
        SkaInputActionData* actionData = &inputState.inputActionData[deviceIndex][handle];
        // TODO: Shouldn't have to check for NULL name as handle should be enough
        if (actionData->handle == SKA_INPUT_INVALID_INPUT_ACTION_HANDLE || actionData->action.name == NULL) {
            continue;
        }
        if (strcmp(actionData->action.name, actionName) == 0) {
            return actionData->handle;
        }
    }
    return SKA_INPUT_INVALID_INPUT_ACTION_HANDLE;
}

bool ska_input_is_input_action_pressed(SkaInputActionHandle handle, SkaInputDeviceIndex deviceIndex) {
    const SkaInputActionData* actionData = &inputState.inputActionData[deviceIndex][handle];
    for (usize i = 0; i < actionData->action.actionValuesCount; i++) {
        const SkaInputActionValue* actionValue = &actionData->action.actionValues[i];
        if (ska_input_is_key_pressed(actionValue->key, deviceIndex)) {
            return true;
        }
    }
    return false;
}

bool ska_input_is_input_action_just_pressed(SkaInputActionHandle handle, SkaInputDeviceIndex deviceIndex) {
    const SkaInputActionData* actionData = &inputState.inputActionData[deviceIndex][handle];
    for (usize i = 0; i < actionData->action.actionValuesCount; i++) {
        const SkaInputActionValue* actionValue = &actionData->action.actionValues[i];
        if (ska_input_is_key_just_pressed(actionValue->key, deviceIndex)) {
            return true;
        }
    }
    return false;
}

bool ska_input_is_input_action_just_released(SkaInputActionHandle handle, SkaInputDeviceIndex deviceIndex) {
    const SkaInputActionData* actionData = &inputState.inputActionData[deviceIndex][handle];
    for (usize i = 0; i < actionData->action.actionValuesCount; i++) {
        const SkaInputActionValue* actionValue = &actionData->action.actionValues[i];
        if (ska_input_is_key_just_released(actionValue->key, deviceIndex)) {
            return true;
        }
    }
    return false;
}

f32 ska_input_get_input_action_strength(SkaInputActionHandle handle, SkaInputDeviceIndex deviceIndex) {
    const SkaInputActionData* actionData = &inputState.inputActionData[deviceIndex][handle];
    for (usize i = 0; i < actionData->action.actionValuesCount; i++) {
        const SkaInputActionValue* actionValue = &actionData->action.actionValues[i];
        const f32 keyStrength = ska_input_get_key_strength(actionValue->key, deviceIndex);
        if (keyStrength >= actionValue->strengthThreshold) {
            return keyStrength;
        }
    }
    return 0.0f;
}

void ska_input_new_frame() {
    for (usize i = 0; i < inputState.cleanupKeyStateJustPressedCount; i++) {
        const SkaInputStateCleanup* stateCleanup = &inputState.cleanupKeyStateJustPressed[i];
        SkaInputKeyState* keyState = &inputState.inputKeyState[stateCleanup->deviceIndex][stateCleanup->key];
        keyState->isJustPressed = false;
    }
    for (usize i = 0; i < inputState.cleanupKeyStateJustReleasedCount; i++) {
        const SkaInputStateCleanup* stateCleanup = &inputState.cleanupKeyStateJustReleased[i];
        SkaInputKeyState* keyState = &inputState.inputKeyState[stateCleanup->deviceIndex][stateCleanup->key];
        keyState->isJustReleased = false;
    }
    inputState.cleanupKeyStateJustPressedCount = 0;
    inputState.cleanupKeyStateJustReleasedCount = 0;
}

void ska_input_reset_gamepad(SkaInputDeviceIndex deviceIndex) {
    SKA_INPUT_KEY_GAMEPAD_FOR_EACH(key) {
        inputState.inputKeyState[deviceIndex][key] = (SkaInputKeyState){0};
    }
}

void ska_input_reset(SkaInputDeviceIndex deviceIndex) {
    inputState = (SkaInputState)DEFAULT_INPUT_STATE;
}

// Private
SkaInputKey get_key_from_2d_axis_key(SkaInputKey axis2DKey, SkaAxisInputValues* axisInputValues, f32 axisValue) {
    switch (axis2DKey) {
        case SkaInputKey_GAMEPAD_LEFT_ANALOG_2D_AXIS_X: {
            const f32 activeValue = axisInputValues->left.x != 0.0f ? axisInputValues->left.x : axisValue;
            return activeValue > 0 ? SkaInputKey_GAMEPAD_LEFT_ANALOG_RIGHT : SkaInputKey_GAMEPAD_LEFT_ANALOG_LEFT;
        }
        case SkaInputKey_GAMEPAD_LEFT_ANALOG_2D_AXIS_Y: {
            const f32 activeValue = axisInputValues->left.y != 0.0f ? axisInputValues->left.y : axisValue;
            return activeValue > 0 ? SkaInputKey_GAMEPAD_LEFT_ANALOG_DOWN : SkaInputKey_GAMEPAD_LEFT_ANALOG_UP;
        }
        case SkaInputKey_GAMEPAD_RIGHT_ANALOG_2D_AXIS_X: {
            const f32 activeValue = axisInputValues->right.x != 0.0f ? axisInputValues->right.x : axisValue;
            return activeValue > 0 ? SkaInputKey_GAMEPAD_RIGHT_ANALOG_RIGHT : SkaInputKey_GAMEPAD_RIGHT_ANALOG_LEFT;
        }
        case SkaInputKey_GAMEPAD_RIGHT_ANALOG_2D_AXIS_Y: {
            const f32 activeValue = axisInputValues->right.y != 0.0f ? axisInputValues->right.y : axisValue;
            return activeValue > 0 ? SkaInputKey_GAMEPAD_RIGHT_ANALOG_DOWN : SkaInputKey_GAMEPAD_RIGHT_ANALOG_UP;
        }
        default: {
            SKA_ASSERT_FMT(false, "Not a valid axis input key!");
            return SkaInputKey_INVALID;
        }
    }
}

void set_prev_input_axis_value(SkaInputKey key, SkaAxisInputValues* axisInputValues, f32 axisValue) {
    switch (key) {
        case SkaInputKey_GAMEPAD_LEFT_ANALOG_2D_AXIS_X: {
            axisInputValues->left.x = axisValue;
            break;
        }
        case SkaInputKey_GAMEPAD_LEFT_ANALOG_2D_AXIS_Y: {
            axisInputValues->left.y = axisValue;
            break;
        }
        case SkaInputKey_GAMEPAD_RIGHT_ANALOG_2D_AXIS_X: {
            axisInputValues->right.x = axisValue;
            break;
        }
        case SkaInputKey_GAMEPAD_RIGHT_ANALOG_2D_AXIS_Y: {
            axisInputValues->right.y = axisValue;
            break;
        }
        default: {
            SKA_ASSERT_FMT(false, "Not a valid axis input key!");
        }
    }
}

void update_axis_key_state(SkaInputKey key, SkaInputInteractionStatus interactionStatus, f32 axisValue, SkaInputDeviceIndex deviceIndex) {
    SkaAxisInputValues* prevAxisInput = &inputState.prevAxisInputs[deviceIndex];
    const SkaInputKey axisKey = get_key_from_2d_axis_key(key, prevAxisInput, axisValue);
    SkaInputKeyState* axisKeyState = &inputState.inputKeyState[deviceIndex][axisKey];
    switch (interactionStatus) {
        case SkaInputInteractionStatus_AXIS_STARTED_MOTION:
        case SkaInputInteractionStatus_AXIS_IN_MOTION: {
            axisKeyState->isPressed = true;
            if (interactionStatus == SkaInputInteractionStatus_AXIS_STARTED_MOTION) {
                axisKeyState->isJustPressed = true;
                SKA_ASSERT(inputState.cleanupKeyStateJustPressedCount + 1 <= SKA_INPUT_MAX_KEY_STATE_CLEANUP_SIZE);
                inputState.cleanupKeyStateJustPressed[inputState.cleanupKeyStateJustPressedCount++] = (SkaInputStateCleanup) { .key = key, .deviceIndex = deviceIndex };
            } else {
                axisKeyState->isJustPressed = false;
            }
            axisKeyState->isJustReleased = false;
            axisKeyState->strength = (f32)fabs(axisValue);
            set_prev_input_axis_value(key, prevAxisInput, axisValue);
            break;
        }
        case SkaInputInteractionStatus_AXIS_STOPPED_MOTION: {
            axisKeyState->isPressed = false;
            axisKeyState->isJustPressed = false;
            axisKeyState->isJustReleased = true;
            axisKeyState->strength = 0.0f;
            set_prev_input_axis_value(key, prevAxisInput, 0.0f);
            SKA_ASSERT(inputState.cleanupKeyStateJustReleasedCount + 1 <= SKA_INPUT_MAX_KEY_STATE_CLEANUP_SIZE);
            inputState.cleanupKeyStateJustReleased[inputState.cleanupKeyStateJustReleasedCount++] = (SkaInputStateCleanup) { .key = key, .deviceIndex = deviceIndex };
            break;
        }
        default: {
            SKA_ASSERT_FMT(false, "Invalid interaction status fo 2d axis key when trying to update key state!");
        }
    }
}
