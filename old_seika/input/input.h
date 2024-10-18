#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "seika/math/math.h"

typedef int32 SkaInputDeviceIndex;
typedef uint32 SkaInputSubscriberHandle;
typedef uint32 SkaInputActionHandle;
typedef float SkaInputKeyStrength;

#define SKA_INPUT_MAX_DEVICES ((SkaInputDeviceIndex)8)
#define SKA_INPUT_FIRST_PLAYER_DEVICE_INDEX ((SkaInputDeviceIndex)0)
#define SKA_INPUT_MAX_INPUT_ACTIONS ((SkaInputActionHandle)128)
#define SKA_INPUT_INVALID_INPUT_ACTION_HANDLE ((SkaInputActionHandle)-1)
#define SKA_INPUT_MAX_INPUT_ACTION_VALUES 8

typedef enum SkaInputSourceType {
    SkaInputSourceType_INVALID,
    SkaInputSourceType_KEYBOARD,
    SkaInputSourceType_GAMEPAD,
    SkaInputSourceType_MOUSE,
} SkaInputSourceType;

typedef enum SkaInputTriggerType {
    SkaInputTriggerType_INVALID,
    SkaInputTriggerType_PRESSED,
    SkaInputTriggerType_RELEASED,
    SkaInputTriggerType_AXIS_IN_MOTION,
    SkaInputTriggerType_AXIS_STOPPED_MOTION,
    SkaInputTriggerType_DEVICE_ADDED,
    SkaInputTriggerType_DEVICE_REMOVED,
} SkaInputTriggerType;

typedef enum SkaInputInteractionStatus {
    SkaInputInteractionStatus_PRESSED,
    SkaInputInteractionStatus_JUST_PRESSED,
    SkaInputInteractionStatus_JUST_RELEASED,
    SkaInputInteractionStatus_AXIS_STARTED_MOTION,
    SkaInputInteractionStatus_AXIS_IN_MOTION,
    SkaInputInteractionStatus_AXIS_STOPPED_MOTION,
} SkaInputInteractionStatus;

typedef enum SkaInputKey {
    SkaInputKey_INVALID,
    // Gamepad
    SkaInputKey_GAMEPAD_DPAD_DOWN,
    SkaInputKey_GAMEPAD_DPAD_UP,
    SkaInputKey_GAMEPAD_DPAD_LEFT,
    SkaInputKey_GAMEPAD_DPAD_RIGHT,
    SkaInputKey_GAMEPAD_FACE_BUTTON_NORTH,  // XBOX Y
    SkaInputKey_GAMEPAD_FACE_BUTTON_SOUTH,  // XBOX A
    SkaInputKey_GAMEPAD_FACE_BUTTON_EAST,   // XBOX B
    SkaInputKey_GAMEPAD_FACE_BUTTON_WEST,   // XBOX X
    SkaInputKey_GAMEPAD_START,
    SkaInputKey_GAMEPAD_BACK,
    SkaInputKey_GAMEPAD_LEFT_SHOULDER,      // PS L1
    SkaInputKey_GAMEPAD_LEFT_TRIGGER,       // PS L2
    SkaInputKey_GAMEPAD_LEFT_ANALOG_BUTTON, // PS L3
    SkaInputKey_GAMEPAD_LEFT_ANALOG_2D_AXIS_X,
    SkaInputKey_GAMEPAD_LEFT_ANALOG_2D_AXIS_Y,
    SkaInputKey_GAMEPAD_RIGHT_SHOULDER,      // PS R1
    SkaInputKey_GAMEPAD_RIGHT_TRIGGER,       // PS R2
    SkaInputKey_GAMEPAD_RIGHT_ANALOG_BUTTON, // PS R3
    SkaInputKey_GAMEPAD_RIGHT_ANALOG_2D_AXIS_X,
    SkaInputKey_GAMEPAD_RIGHT_ANALOG_2D_AXIS_Y,
    // Keeping these separate as 'RegisterInputEvent' should only use 'SkaInputKey_GAMEPAD_LEFT_ANALOG_2D_AXIS_X', 'SkaInputKey_GAMEPAD_LEFT_ANALOG_2D_AXIS_Y',
    // 'SkaInputKey_GAMEPAD_RIGHT_ANALOG_2D_AXIS_X', and 'SkaInputKey_GAMEPAD_RIGHT_ANALOG_2D_AXIS_Y'.
    // The key states for these keys will be updated automatically by the input system.
    SkaInputKey_GAMEPAD_LEFT_ANALOG_LEFT,
    SkaInputKey_GAMEPAD_LEFT_ANALOG_RIGHT,
    SkaInputKey_GAMEPAD_LEFT_ANALOG_UP,
    SkaInputKey_GAMEPAD_LEFT_ANALOG_DOWN,
    SkaInputKey_GAMEPAD_RIGHT_ANALOG_LEFT,
    SkaInputKey_GAMEPAD_RIGHT_ANALOG_RIGHT,
    SkaInputKey_GAMEPAD_RIGHT_ANALOG_UP,
    SkaInputKey_GAMEPAD_RIGHT_ANALOG_DOWN,

    // Keyboard
    SkaInputKey_KEYBOARD_TAB,
    SkaInputKey_KEYBOARD_LEFT,
    SkaInputKey_KEYBOARD_RIGHT,
    SkaInputKey_KEYBOARD_UP,
    SkaInputKey_KEYBOARD_DOWN,
    SkaInputKey_KEYBOARD_PAGE_DOWN,
    SkaInputKey_KEYBOARD_PAGE_UP,
    SkaInputKey_KEYBOARD_HOME,
    SkaInputKey_KEYBOARD_END,
    SkaInputKey_KEYBOARD_INSERT,
    SkaInputKey_KEYBOARD_DELETE,
    SkaInputKey_KEYBOARD_BACKSPACE,
    SkaInputKey_KEYBOARD_SPACE,
    SkaInputKey_KEYBOARD_RETURN,
    SkaInputKey_KEYBOARD_ESCAPE,
    SkaInputKey_KEYBOARD_QUOTE,
    SkaInputKey_KEYBOARD_COMMA,
    SkaInputKey_KEYBOARD_MINUS,
    SkaInputKey_KEYBOARD_PERIOD,
    SkaInputKey_KEYBOARD_SLASH,
    SkaInputKey_KEYBOARD_SEMICOLON,
    SkaInputKey_KEYBOARD_EQUALS,
    SkaInputKey_KEYBOARD_LEFT_BRACKET,
    SkaInputKey_KEYBOARD_RIGHT_BRACKET,
    SkaInputKey_KEYBOARD_BACKSLASH,
    SkaInputKey_KEYBOARD_BACKQUOTE,
    SkaInputKey_KEYBOARD_CAPS_LOCK,
    SkaInputKey_KEYBOARD_SCROLL_LOCK,
    SkaInputKey_KEYBOARD_NUM_LOCK_CLEAR,
    SkaInputKey_KEYBOARD_PRINT_SCREEN,
    SkaInputKey_KEYBOARD_PAUSE,
    SkaInputKey_KEYBOARD_KEYPAD_0,
    SkaInputKey_KEYBOARD_KEYPAD_1,
    SkaInputKey_KEYBOARD_KEYPAD_2,
    SkaInputKey_KEYBOARD_KEYPAD_3,
    SkaInputKey_KEYBOARD_KEYPAD_4,
    SkaInputKey_KEYBOARD_KEYPAD_5,
    SkaInputKey_KEYBOARD_KEYPAD_6,
    SkaInputKey_KEYBOARD_KEYPAD_7,
    SkaInputKey_KEYBOARD_KEYPAD_8,
    SkaInputKey_KEYBOARD_KEYPAD_9,
    SkaInputKey_KEYBOARD_KEYPAD_PERIOD,
    SkaInputKey_KEYBOARD_KEYPAD_DIVIDE,
    SkaInputKey_KEYBOARD_KEYPAD_MULTIPLY,
    SkaInputKey_KEYBOARD_KEYPAD_MINUS,
    SkaInputKey_KEYBOARD_KEYPAD_PLUS,
    SkaInputKey_KEYBOARD_KEYPAD_ENTER,
    SkaInputKey_KEYBOARD_KEYPAD_EQUALS,
    SkaInputKey_KEYBOARD_LEFT_CONTROL,
    SkaInputKey_KEYBOARD_LEFT_SHIFT,
    SkaInputKey_KEYBOARD_LEFT_ALT,
    SkaInputKey_KEYBOARD_LEFT_GUI,
    SkaInputKey_KEYBOARD_RIGHT_CONTROL,
    SkaInputKey_KEYBOARD_RIGHT_SHIFT,
    SkaInputKey_KEYBOARD_RIGHT_ALT,
    SkaInputKey_KEYBOARD_RIGHT_GUI,
    SkaInputKey_KEYBOARD_APPLICATION,
    SkaInputKey_KEYBOARD_NUM_0,
    SkaInputKey_KEYBOARD_NUM_1,
    SkaInputKey_KEYBOARD_NUM_2,
    SkaInputKey_KEYBOARD_NUM_3,
    SkaInputKey_KEYBOARD_NUM_4,
    SkaInputKey_KEYBOARD_NUM_5,
    SkaInputKey_KEYBOARD_NUM_6,
    SkaInputKey_KEYBOARD_NUM_7,
    SkaInputKey_KEYBOARD_NUM_8,
    SkaInputKey_KEYBOARD_NUM_9,
    SkaInputKey_KEYBOARD_A,
    SkaInputKey_KEYBOARD_B,
    SkaInputKey_KEYBOARD_C,
    SkaInputKey_KEYBOARD_D,
    SkaInputKey_KEYBOARD_E,
    SkaInputKey_KEYBOARD_F,
    SkaInputKey_KEYBOARD_G,
    SkaInputKey_KEYBOARD_H,
    SkaInputKey_KEYBOARD_I,
    SkaInputKey_KEYBOARD_J,
    SkaInputKey_KEYBOARD_K,
    SkaInputKey_KEYBOARD_L,
    SkaInputKey_KEYBOARD_M,
    SkaInputKey_KEYBOARD_N,
    SkaInputKey_KEYBOARD_O,
    SkaInputKey_KEYBOARD_P,
    SkaInputKey_KEYBOARD_Q,
    SkaInputKey_KEYBOARD_R,
    SkaInputKey_KEYBOARD_S,
    SkaInputKey_KEYBOARD_T,
    SkaInputKey_KEYBOARD_U,
    SkaInputKey_KEYBOARD_V,
    SkaInputKey_KEYBOARD_W,
    SkaInputKey_KEYBOARD_X,
    SkaInputKey_KEYBOARD_Y,
    SkaInputKey_KEYBOARD_Z,
    SkaInputKey_KEYBOARD_F1,
    SkaInputKey_KEYBOARD_F2,
    SkaInputKey_KEYBOARD_F3,
    SkaInputKey_KEYBOARD_F4,
    SkaInputKey_KEYBOARD_F5,
    SkaInputKey_KEYBOARD_F6,
    SkaInputKey_KEYBOARD_F7,
    SkaInputKey_KEYBOARD_F8,
    SkaInputKey_KEYBOARD_F9,
    SkaInputKey_KEYBOARD_F10,
    SkaInputKey_KEYBOARD_F11,
    SkaInputKey_KEYBOARD_F12,
    SkaInputKey_KEYBOARD_F13,
    SkaInputKey_KEYBOARD_F14,
    SkaInputKey_KEYBOARD_F15,
    SkaInputKey_KEYBOARD_F16,
    SkaInputKey_KEYBOARD_F17,
    SkaInputKey_KEYBOARD_F18,
    SkaInputKey_KEYBOARD_F19,
    SkaInputKey_KEYBOARD_F20,
    SkaInputKey_KEYBOARD_F21,
    SkaInputKey_KEYBOARD_F22,
    SkaInputKey_KEYBOARD_F23,
    SkaInputKey_KEYBOARD_F24,
    SkaInputKey_KEYBOARD_APP_FORWARD,
    SkaInputKey_KEYBOARD_APP_BACK,

    // Mouse
    SkaInputKey_MOUSE_BUTTON_LEFT,
    SkaInputKey_MOUSE_BUTTON_RIGHT,
    SkaInputKey_MOUSE_BUTTON_MIDDLE,

    SkaInputKey_NUMBER_OF_KEYS
} SkaInputKey;

#define SKA_INPUT_KEY_GAMEPAD_ENUM_START SkaInputKey_GAMEPAD_DPAD_DOWN
#define SKA_INPUT_KEY_GAMEPAD_ENUM_END SkaInputKey_GAMEPAD_RIGHT_ANALOG_DOWN
#define SKA_INPUT_KEY_KEYBOARD_ENUM_START SkaInputKey_KEYBOARD_TAB
#define SKA_INPUT_KEY_KEYBOARD_ENUM_END SkaInputKey_KEYBOARD_APP_BACK
#define SKA_INPUT_KEY_MOUSE_ENUM_START SkaInputKey_MOUSE_BUTTON_LEFT
#define SKA_INPUT_KEY_MOUSE_ENUM_END SkaInputKey_MOUSE_BUTTON_MIDDLE

#define SKA_INPUT_KEY_GAMEPAD_COUNT (SKA_INPUT_KEY_GAMEPAD_ENUM_END - SKA_INPUT_KEY_GAMEPAD_ENUM_START)
#define SKA_INPUT_KEY_KEYBOARD_COUNT (SKA_INPUT_KEY_KEYBOARD_ENUM_END - SKA_INPUT_KEY_KEYBOARD_ENUM_START)
#define SKA_INPUT_KEY_MOUSE_COUNT (SKA_INPUT_KEY_MOUSE_ENUM_END - SKA_INPUT_KEY_MOUSE_ENUM_START)

#define SKA_INPUT_KEY_GAMEPAD_FOR_EACH(KeyName) for(SkaInputKey KeyName = SKA_INPUT_KEY_GAMEPAD_ENUM_START; KeyName <= SKA_INPUT_KEY_GAMEPAD_ENUM_END; KeyName++)
#define SKA_INPUT_KEY_KEYBOARD_FOR_EACH(KeyName) for(SkaInputKey KeyName = SKA_INPUT_KEY_KEYBOARD_ENUM_START; KeyName <= SKA_INPUT_KEY_KEYBOARD_ENUM_END; KeyName++)
#define SKA_INPUT_KEY_MOUSE_FOR_EACH(KeyName) for(SkaInputKey KeyName = SKA_INPUT_KEY_MOUSE_ENUM_START; KeyName <= SKA_INPUT_KEY_MOUSE_ENUM_END; KeyName++)

typedef enum SkaInputAxis {
    SkaInputAxis_LEFT,
    SkaInputAxis_RIGHT,
} SkaInputAxis;

typedef struct SkaMouse {
    SkaVector2 position;
} SkaMouse;

typedef struct SkaInputEvent {
    SkaInputSourceType sourceType;
    SkaInputInteractionStatus interactionStatus;
    SkaInputKey key;
    SkaInputDeviceIndex deviceIndex;
    f32 gamepadAxisValue;
} SkaInputEvent;

typedef struct SkaInputActionValue {
    SkaInputKey key;
    SkaInputKeyStrength strengthThreshold;
} SkaInputActionValue;

typedef struct SkaInputAction {
    char* name;
    SkaInputActionValue actionValues[SKA_INPUT_MAX_INPUT_ACTION_VALUES];
    usize actionValuesCount;
} SkaInputAction;

const char* ska_input_key_to_string(SkaInputKey key);
SkaInputKey ska_input_string_to_key(const char* keyName);

bool ska_input_is_keyboard_key(SkaInputKey key);
bool ska_input_is_mouse_key(SkaInputKey key);
bool ska_input_is_gamepad_key(SkaInputKey key);

// Events
void ska_input_register_gamepad_added_event(SkaInputDeviceIndex deviceIndex);
void ska_input_register_gamepad_removed_event(SkaInputDeviceIndex deviceIndex);
void ska_input_register_input_event(SkaInputSourceType sourceType, SkaInputKey key, SkaInputTriggerType triggerType, SkaInputDeviceIndex deviceIndex, f32 gamepadAxisValue);
void ska_input_register_input_event2(SkaInputSourceType sourceType, SkaInputKey key, SkaInputTriggerType triggerType, SkaInputDeviceIndex deviceIndex);
void ska_input_register_input_event3(SkaInputSourceType sourceType, SkaInputKey key, SkaInputTriggerType triggerType);

bool ska_input_is_key_pressed(SkaInputKey key, SkaInputDeviceIndex deviceIndex);
bool ska_input_is_key_just_pressed(SkaInputKey key, SkaInputDeviceIndex deviceIndex);
bool ska_input_is_key_just_released(SkaInputKey key, SkaInputDeviceIndex deviceIndex);
f32 ska_input_get_key_strength(SkaInputKey key, SkaInputDeviceIndex deviceIndex);
SkaVector2 ska_input_get_axis_input(SkaInputAxis axis, SkaInputDeviceIndex deviceIndex);

SkaMouse* ska_input_get_mouse();

// Input Action
SkaInputActionHandle ska_input_add_input_action(const char* actionName, const SkaInputActionValue* actionValues, SkaInputDeviceIndex deviceIndex);
SkaInputAction* ska_input_get_input_action(SkaInputActionHandle handle, SkaInputDeviceIndex deviceIndex);
bool ska_input_remove_input_action(SkaInputActionHandle handle, SkaInputDeviceIndex deviceIndex);
SkaInputActionHandle ska_input_find_input_action_handle(const char* actionName, SkaInputDeviceIndex deviceIndex);

bool ska_input_is_input_action_pressed(SkaInputActionHandle handle, SkaInputDeviceIndex deviceIndex);
bool ska_input_is_input_action_just_pressed(SkaInputActionHandle handle, SkaInputDeviceIndex deviceIndex);
bool ska_input_is_input_action_just_released(SkaInputActionHandle handle, SkaInputDeviceIndex deviceIndex);
f32 ska_input_get_input_action_strength(SkaInputActionHandle handle, SkaInputDeviceIndex deviceIndex);

void ska_input_new_frame();
void ska_input_reset_gamepad(SkaInputDeviceIndex deviceIndex);
void ska_input_reset(SkaInputDeviceIndex deviceIndex);

#ifdef __cplusplus
}
#endif
