#include <unity.h>

#include "../seika/input.h"

#define RESOURCES_PATH "test/resources"
#define RESOURCES_PACK_PATH "test/resources/test.pck"

void setUp() {}
void tearDown() {}

void seika_input_test(void);

int main(int argv, char** args) {
    UNITY_BEGIN();
    RUN_TEST(seika_input_test);
    return UNITY_END();
}

void seika_input_test(void) {
    TEST_MESSAGE("Testing 'for each' and checking input type");
    SKA_INPUT_KEY_GAMEPAD_FOR_EACH(key) {
        TEST_ASSERT_TRUE(ska_input_is_gamepad_key(key));
    }
    SKA_INPUT_KEY_KEYBOARD_FOR_EACH(key) {
        TEST_ASSERT_TRUE(ska_input_is_keyboard_key(key));
    }
    SKA_INPUT_KEY_MOUSE_FOR_EACH(key) {
        TEST_ASSERT_TRUE(ska_input_is_mouse_key(key));
    }

    TEST_MESSAGE("Testing events");
    ska_input_register_input_event3(SkaInputSourceType_KEYBOARD, SkaInputKey_KEYBOARD_BACKSPACE, SkaInputTriggerType_PRESSED);
    TEST_ASSERT_TRUE(ska_input_is_key_pressed(SkaInputKey_KEYBOARD_BACKSPACE, SKA_INPUT_FIRST_PLAYER_DEVICE_INDEX));
    TEST_ASSERT_TRUE(ska_input_is_key_just_pressed(SkaInputKey_KEYBOARD_BACKSPACE, SKA_INPUT_FIRST_PLAYER_DEVICE_INDEX));
    TEST_ASSERT_FALSE(ska_input_is_key_just_released(SkaInputKey_KEYBOARD_BACKSPACE, SKA_INPUT_FIRST_PLAYER_DEVICE_INDEX));

    ska_input_register_input_event3(SkaInputSourceType_KEYBOARD, SkaInputKey_KEYBOARD_BACKSPACE, SkaInputTriggerType_PRESSED);
    TEST_ASSERT_TRUE(ska_input_is_key_pressed(SkaInputKey_KEYBOARD_BACKSPACE, SKA_INPUT_FIRST_PLAYER_DEVICE_INDEX));
    TEST_ASSERT_FALSE(ska_input_is_key_just_pressed(SkaInputKey_KEYBOARD_BACKSPACE, SKA_INPUT_FIRST_PLAYER_DEVICE_INDEX));
    TEST_ASSERT_FALSE(ska_input_is_key_just_released(SkaInputKey_KEYBOARD_BACKSPACE, SKA_INPUT_FIRST_PLAYER_DEVICE_INDEX));

    ska_input_register_input_event3(SkaInputSourceType_KEYBOARD, SkaInputKey_KEYBOARD_BACKSPACE, SkaInputTriggerType_RELEASED);
    TEST_ASSERT_FALSE(ska_input_is_key_pressed(SkaInputKey_KEYBOARD_BACKSPACE, SKA_INPUT_FIRST_PLAYER_DEVICE_INDEX));
    TEST_ASSERT_FALSE(ska_input_is_key_just_pressed(SkaInputKey_KEYBOARD_BACKSPACE, SKA_INPUT_FIRST_PLAYER_DEVICE_INDEX));
    TEST_ASSERT_TRUE(ska_input_is_key_just_released(SkaInputKey_KEYBOARD_BACKSPACE, SKA_INPUT_FIRST_PLAYER_DEVICE_INDEX));

    TEST_MESSAGE("Testing input get axis input");
    ska_input_register_input_event(SkaInputSourceType_GAMEPAD, SkaInputKey_GAMEPAD_LEFT_ANALOG_2D_AXIS_Y, SkaInputTriggerType_AXIS_IN_MOTION, SKA_INPUT_FIRST_PLAYER_DEVICE_INDEX, -0.5f);
    const SkaVector2 firstAxisInput = ska_input_get_axis_input(SkaInputAxis_LEFT, SKA_INPUT_FIRST_PLAYER_DEVICE_INDEX);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, firstAxisInput.x);
    TEST_ASSERT_EQUAL_FLOAT(-0.5f, firstAxisInput.y);

    ska_input_register_input_event(SkaInputSourceType_GAMEPAD, SkaInputKey_GAMEPAD_LEFT_ANALOG_2D_AXIS_X, SkaInputTriggerType_AXIS_IN_MOTION, SKA_INPUT_FIRST_PLAYER_DEVICE_INDEX, 0.25f);
    const SkaVector2 secondAxisInput = ska_input_get_axis_input(SkaInputAxis_LEFT, SKA_INPUT_FIRST_PLAYER_DEVICE_INDEX);
    TEST_ASSERT_EQUAL_FLOAT(0.25f, secondAxisInput.x);
    TEST_ASSERT_EQUAL_FLOAT(-0.5f, secondAxisInput.y);

    ska_input_register_input_event(SkaInputSourceType_GAMEPAD, SkaInputKey_GAMEPAD_LEFT_ANALOG_2D_AXIS_X, SkaInputTriggerType_AXIS_STOPPED_MOTION, SKA_INPUT_FIRST_PLAYER_DEVICE_INDEX, 0.25f);
    ska_input_register_input_event(SkaInputSourceType_GAMEPAD, SkaInputKey_GAMEPAD_LEFT_ANALOG_2D_AXIS_Y, SkaInputTriggerType_AXIS_STOPPED_MOTION, SKA_INPUT_FIRST_PLAYER_DEVICE_INDEX, 0.25f);
    const SkaVector2 thirdAxisInput = ska_input_get_axis_input(SkaInputAxis_LEFT, SKA_INPUT_FIRST_PLAYER_DEVICE_INDEX);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, thirdAxisInput.x);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, thirdAxisInput.y);

    ska_input_register_input_event(SkaInputSourceType_GAMEPAD, SkaInputKey_GAMEPAD_RIGHT_ANALOG_2D_AXIS_X, SkaInputTriggerType_AXIS_IN_MOTION, SKA_INPUT_FIRST_PLAYER_DEVICE_INDEX, 1.0f);
    const SkaVector2 fourthAxisInput = ska_input_get_axis_input(SkaInputAxis_RIGHT, SKA_INPUT_FIRST_PLAYER_DEVICE_INDEX);
    TEST_ASSERT_EQUAL_FLOAT(1.0f, fourthAxisInput.x);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, fourthAxisInput.y);
}
