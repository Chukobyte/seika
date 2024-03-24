#include <unity.h>

#include "seika/input/input.h"
#include "seika/data_structures/hash_map.h"
#include "seika/data_structures/spatial_hash_map.h"
#include "seika/data_structures/array2d.h"
#include "seika/data_structures/linked_list.h"
#include "seika/memory.h"

#define RESOURCES_PATH "test/resources"
#define RESOURCES_PACK_PATH "test/resources/test.pck"

void setUp(void) {}
void tearDown(void) {}

void seika_input_test(void);
void seika_hash_map_test(void);
void seika_spatial_hash_map_test(void);
void seika_linked_list_test(void);
void seika_array2d_test(void);

int32 main(int32 argv, char** args) {
    UNITY_BEGIN();
    RUN_TEST(seika_input_test);
    RUN_TEST(seika_hash_map_test);
    RUN_TEST(seika_spatial_hash_map_test);
    RUN_TEST(seika_linked_list_test);
    RUN_TEST(seika_array2d_test);
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

    TEST_MESSAGE("Testing input actions");
    SkaInputActionHandle jumpActionHandle = ska_input_add_input_action(
        "jump",
        (SkaInputActionValue[]){ { .key = SkaInputKey_KEYBOARD_SPACE, .strengthThreshold = 0.5f }, { .key = SkaInputKey_KEYBOARD_RETURN, .strengthThreshold = 0.5f }, { SkaInputKey_INVALID } },
        SKA_INPUT_FIRST_PLAYER_DEVICE_INDEX
    );
    TEST_ASSERT_NOT_EQUAL_UINT32(SKA_INPUT_INVALID_INPUT_ACTION_HANDLE, jumpActionHandle);
    jumpActionHandle = ska_input_find_input_action_handle("jump", SKA_INPUT_FIRST_PLAYER_DEVICE_INDEX);
    TEST_ASSERT_NOT_EQUAL_UINT32(SKA_INPUT_INVALID_INPUT_ACTION_HANDLE, jumpActionHandle);

    ska_input_register_input_event(SkaInputSourceType_KEYBOARD, SkaInputKey_KEYBOARD_RETURN, SkaInputTriggerType_PRESSED, SKA_INPUT_FIRST_PLAYER_DEVICE_INDEX, 0.0f);
    TEST_ASSERT_TRUE(ska_input_is_input_action_pressed(jumpActionHandle, SKA_INPUT_FIRST_PLAYER_DEVICE_INDEX));
    TEST_ASSERT_TRUE(ska_input_is_input_action_just_pressed(jumpActionHandle, SKA_INPUT_FIRST_PLAYER_DEVICE_INDEX));
    TEST_ASSERT_FALSE(ska_input_is_input_action_just_released(jumpActionHandle, SKA_INPUT_FIRST_PLAYER_DEVICE_INDEX));
    TEST_ASSERT_EQUAL_FLOAT(1.0f, ska_input_get_input_action_strength(jumpActionHandle, SKA_INPUT_FIRST_PLAYER_DEVICE_INDEX));

    ska_input_new_frame();
    TEST_ASSERT_TRUE(ska_input_is_input_action_pressed(jumpActionHandle, SKA_INPUT_FIRST_PLAYER_DEVICE_INDEX));
    TEST_ASSERT_FALSE(ska_input_is_input_action_just_pressed(jumpActionHandle, SKA_INPUT_FIRST_PLAYER_DEVICE_INDEX));
    TEST_ASSERT_FALSE(ska_input_is_input_action_just_released(jumpActionHandle, SKA_INPUT_FIRST_PLAYER_DEVICE_INDEX));

    ska_input_register_input_event(SkaInputSourceType_KEYBOARD, SkaInputKey_KEYBOARD_RETURN, SkaInputTriggerType_RELEASED, SKA_INPUT_FIRST_PLAYER_DEVICE_INDEX, 0.0f);
    TEST_ASSERT_FALSE(ska_input_is_input_action_pressed(jumpActionHandle, SKA_INPUT_FIRST_PLAYER_DEVICE_INDEX));
    TEST_ASSERT_FALSE(ska_input_is_input_action_just_pressed(jumpActionHandle, SKA_INPUT_FIRST_PLAYER_DEVICE_INDEX));
    TEST_ASSERT_TRUE(ska_input_is_input_action_just_released(jumpActionHandle, SKA_INPUT_FIRST_PLAYER_DEVICE_INDEX));

    ska_input_remove_input_action(jumpActionHandle, SKA_INPUT_FIRST_PLAYER_DEVICE_INDEX);
    TEST_ASSERT_EQUAL_UINT32(SKA_INPUT_INVALID_INPUT_ACTION_HANDLE, ska_input_find_input_action_handle("jump", SKA_INPUT_FIRST_PLAYER_DEVICE_INDEX));
}

void seika_hash_map_test(void) {
    SkaHashMap* hashMap = ska_hash_map_create(sizeof(int32), sizeof(int32), SKA_HASH_MAP_MIN_CAPACITY);
    TEST_ASSERT_NOT_NULL(hashMap);

    int32 key1 = 0;
    int32 value1 = 11;
    ska_hash_map_add(hashMap, &key1, &value1);
    TEST_ASSERT_EQUAL_INT(1, hashMap->size);
    int32 returnedValue1 = *(int32*) ska_hash_map_get(hashMap, &key1);
    TEST_ASSERT_EQUAL_INT(value1, returnedValue1);

    int32 key2 = 1;
    int32 value2 = 22;
    ska_hash_map_add(hashMap, &key2, &value2);
    TEST_ASSERT_EQUAL_INT(2, hashMap->size);

    // Iterator test
    int32 iterCount = 0;
    for (SkaHashMapIterator iterator = ska_hash_map_iter_create(hashMap); ska_hash_map_iter_is_valid(hashMap, &iterator); ska_hash_map_iter_advance(hashMap, &iterator)) {
        iterCount++;
    }
    TEST_ASSERT_EQUAL_INT(2, iterCount);
    // Iter Macro test
    iterCount = 0;
    SKA_HASH_MAP_FOR_EACH(hashMap, iter) {
        iterCount++;
    }
    TEST_ASSERT_EQUAL_INT(2, iterCount);

    ska_hash_map_destroy(hashMap);
}

void seika_spatial_hash_map_test(void) {
    const int32 maxSpriteSize = 32;
    SkaSpatialHashMap* spatialHashMap = ska_spatial_hash_map_create(maxSpriteSize * 2);
    TEST_ASSERT_NOT_NULL(spatialHashMap);

    // Create two entities and insert them into hash map
    const uint32 entity = 1;
    SkaSpatialHashMapGridSpacesHandle* handle = ska_spatial_hash_map_insert_or_update(spatialHashMap, entity, &(SkaRect2) {
        0.0f, 0.0f, 32.0f, 32.0f
    });
    TEST_ASSERT_EQUAL(handle, ska_spatial_hash_map_get(spatialHashMap, entity));

    const uint32 entityTwo = 2;
    SkaSpatialHashMapGridSpacesHandle* handleTwo = ska_spatial_hash_map_insert_or_update(spatialHashMap, entityTwo, &(SkaRect2) {
        16.0f, 16.0f, 48.0f, 48.0f
    });
    TEST_ASSERT_EQUAL(handleTwo, ska_spatial_hash_map_get(spatialHashMap, entityTwo));

    // An entity that should not be collided with
    const uint32 entityNotCollided = 3;
    ska_spatial_hash_map_insert_or_update(spatialHashMap, entityNotCollided, &(SkaRect2) {
        64.0f, 64.0f, 96.0f, 96.0f
    });

    // Test collision result to make sure the two entities collide
    const SkaSpatialHashMapCollisionResult collisionResult = ska_spatial_hash_map_compute_collision(spatialHashMap, entity);
    TEST_ASSERT_EQUAL_INT(1, collisionResult.collisionCount);

    if (collisionResult.collisionCount > 0) {
        TEST_ASSERT_EQUAL_INT(2, collisionResult.collisions[0]);
    }

    ska_spatial_hash_map_remove(spatialHashMap, entity);
    TEST_ASSERT_NULL(ska_spatial_hash_map_get(spatialHashMap, entity));
    ska_spatial_hash_map_remove(spatialHashMap, entityTwo);
    TEST_ASSERT_NULL(ska_spatial_hash_map_get(spatialHashMap, entityTwo));

    ska_spatial_hash_map_destroy(spatialHashMap);
}

static bool linked_list_test_sort(void* a, void* b) {
    int numA = (int) *(int*) a;
    int numB = (int) *(int*) b;
    return numA > numB;
}

void seika_linked_list_test(void) {
    SkaLinkedList* list = ska_linked_list_create(sizeof(int32));
    // Test prepend
    int32 num1 = 9;
    int32 num2 = 7;
    ska_linked_list_prepend(list, &num1);
    const int32 returnedNum = (int32) *(int32*) ska_linked_list_get_front(list);
    TEST_ASSERT_EQUAL_INT(1, list->size);
    TEST_ASSERT_EQUAL_INT(num1, returnedNum);
    ska_linked_list_prepend(list, &num2);
    TEST_ASSERT_EQUAL_INT(2, list->size);
    ska_linked_list_remove(list, &num1);
    TEST_ASSERT_EQUAL_INT(1, list->size);
    int32* returnedNum2 = (int32*) ska_linked_list_pop_front(list);
    TEST_ASSERT_EQUAL_INT(7, *returnedNum2);
    TEST_ASSERT_TRUE(ska_linked_list_is_empty(list));
    SKA_MEM_FREE(returnedNum2);

    int32 numbers[5];
    for (int32 i = 0; i < 5; i++) {
        numbers[i] = i;
        ska_linked_list_append(list, &i);
    }
    TEST_ASSERT_EQUAL_INT(5, list->size);

    // For each loop test
    SkaLinkedListNode* node = NULL;
    int32 loopIndex = 0;
    SKA_LINKED_LIST_FOR_EACH(list, node) {
        const int32 indexNum = (int32)*(int32*)node->value;
        TEST_ASSERT_EQUAL_INT(indexNum, numbers[loopIndex]);
        loopIndex++;
    }

    const int32 indexThreeNum = (int32)*(int32*)ska_linked_list_get(list, 3);
    TEST_ASSERT_EQUAL_INT(3, indexThreeNum);

    // Insert Test
    int32 insertNum1 = 10;
    int32 insertNum2 = 20;
    ska_linked_list_insert(list, &insertNum1, 3);
    TEST_ASSERT_EQUAL_INT(6, list->size);
    ska_linked_list_insert(list, &insertNum2, 1);
    TEST_ASSERT_EQUAL_INT(7, list->size);
    const int32 insertNumThree = (int32)*(int32*)ska_linked_list_get(list, 3);
    TEST_ASSERT_EQUAL_INT(10, insertNumThree);
    const int32 insertNumOne = (int32)*(int32*)ska_linked_list_get(list, 1);
    TEST_ASSERT_EQUAL_INT(20, insertNumOne);

    ska_linked_list_sort(list, linked_list_test_sort);
    const int32 smallestNum = (int32)*(int32*)ska_linked_list_get_front(list);
    const int32 highestNum = (int32)*(int32*)ska_linked_list_get_back(list);
    TEST_ASSERT_EQUAL_INT(0, smallestNum);
    TEST_ASSERT_EQUAL_INT(20, highestNum);

    // Clear test
    ska_linked_list_clear(list);
    TEST_ASSERT_EQUAL_INT(0, list->size);

    ska_linked_list_destroy(list);
}

void seika_array2d_test(void) {
    typedef struct TestArrayStruct {
        int value;
    } TestArrayStruct;

    SkaArray2D* array2D = ska_array2d_create(5, 5, sizeof(TestArrayStruct));

    TestArrayStruct* struct0_3 = (TestArrayStruct*)ska_array2d_get(array2D, 0, 3);
    TEST_ASSERT_NOT_NULL(struct0_3);
    TEST_ASSERT_EQUAL_INT(0, struct0_3->value);

    struct0_3->value = 5;
    const TestArrayStruct* struct0_3Again = (TestArrayStruct*)ska_array2d_get(array2D, 0, 3);
    TEST_ASSERT_EQUAL_INT(5, struct0_3Again->value);

    TestArrayStruct* struct4_2 = (TestArrayStruct*)ska_array2d_get(array2D, 4, 2);
    TEST_ASSERT_NOT_NULL(struct4_2);
    TEST_ASSERT_EQUAL_INT(0, struct4_2->value);
    bool wasSetSuccessful = ska_array2d_set(array2D, 4, 2, &(TestArrayStruct){ .value = 1000 });
    TEST_ASSERT_TRUE(wasSetSuccessful);
    TEST_ASSERT_EQUAL_INT(1000, struct4_2->value);
    wasSetSuccessful = false;

    TestArrayStruct* struct7_9 = (TestArrayStruct*)ska_array2d_get(array2D, 7, 7);
    TEST_ASSERT_NULL(struct7_9);
    wasSetSuccessful = ska_array2d_set(array2D, 7, 9, &(TestArrayStruct){ .value = 1000 });
    TEST_ASSERT_FALSE(wasSetSuccessful);
    ska_array2d_resize(array2D, 10, 10);
    struct7_9 = (TestArrayStruct*)ska_array2d_get(array2D, 7, 7);
    TEST_ASSERT_NOT_NULL(struct7_9);
    // Test old struct values
    const TestArrayStruct* struct0_3Again2 = (TestArrayStruct*)ska_array2d_get(array2D, 0, 3);
    const TestArrayStruct* struct4_2Again = (TestArrayStruct*)ska_array2d_get(array2D, 4, 2);
    TEST_ASSERT_EQUAL_INT(5, struct0_3Again2->value);
    TEST_ASSERT_EQUAL_INT(1000, struct4_2Again->value);

    ska_array2d_resize(array2D, 4, 4);
    TEST_ASSERT_NULL(ska_array2d_get(array2D, 7, 7));
    const TestArrayStruct* struct0_3Again3 = (TestArrayStruct*)ska_array2d_get(array2D, 0, 3);
    TEST_ASSERT_NOT_NULL(struct0_3Again3);
    TEST_ASSERT_EQUAL_INT(5, struct0_3Again3->value);

    ska_array2d_reset(array2D);
    const TestArrayStruct* struct0_3Again4 = (TestArrayStruct*)ska_array2d_get(array2D, 0, 3);
    TEST_ASSERT_EQUAL_INT(0, struct0_3Again4->value);
    ska_array2d_reset_default(array2D, &(TestArrayStruct){ .value = 230 });
    const TestArrayStruct* struct0_3Again5 = (TestArrayStruct*)ska_array2d_get(array2D, 0, 3);
    TEST_ASSERT_EQUAL_INT(230, struct0_3Again5->value);

    ska_array2d_destroy(array2D);
}
