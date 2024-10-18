#include <unity.h>
#include <string.h>

#include "seika/memory.h"
#include "seika/asset/asset_file_loader.h"
#include "seika/data_structures/array_list.h"
#include "seika/data_structures/id_queue.h"
#include "seika/math/math.h"

#if SKA_ECS
#include "seika/ecs/ecs.h"
#include "seika/ecs/ec_system.h"
#endif

#if SKA_INPUT
#include "seika/input/input.h"
#endif

#define RESOURCES_PATH "test/resources"
#define RESOURCES_PACK_PATH "test/resources/test.pck"

void setUp(void) {}
void tearDown(void) {}

void seika_mem_test(void);
void seika_array_list_test(void);
void seika_id_queue_test(void);
void seika_asset_file_loader_test(void);

#if SKA_ECS
void seika_ecs_test(void);
#endif

#if SKA_INPUT
void seika_input_test(void);
#endif

int32 main(int32 argv, char** args) {
    UNITY_BEGIN();
    RUN_TEST(seika_mem_test);
    RUN_TEST(seika_array_list_test);
    RUN_TEST(seika_id_queue_test);
    RUN_TEST(seika_asset_file_loader_test);
#if SKA_ECS
    RUN_TEST(seika_ecs_test);
#endif
#if SKA_INPUT
    RUN_TEST(seika_input_test);
#endif
    return UNITY_END();
}

void seika_mem_test(void) {
    int* testInt = SKA_ALLOC(int);
    TEST_ASSERT_NOT_NULL(testInt);
    *testInt = 5;
    TEST_ASSERT_EQUAL_INT(5, *testInt);
    TEST_ASSERT_TRUE(ska_mem_report_leaks());
    SKA_FREE(testInt);
    TEST_ASSERT_FALSE(ska_mem_report_leaks());
}

void seika_array_list_test(void) {
    int listValues[] = { 8, 2, 3, 5 };
    SkaArrayList* arrayList = ska_array_list_create_default_capacity(sizeof(int));
    TEST_ASSERT_TRUE(ska_array_list_is_empty(arrayList));

    for (int i = 0; i < 4; i++) {
        ska_array_list_push_back(arrayList, &listValues[i]);
    }
    TEST_ASSERT_FALSE(ska_array_list_is_empty(arrayList));
    int index = 0;
    SKA_ARRAY_LIST_FOR_EACH(arrayList, int, i) {
        const int value = (int)*(int*)ska_array_list_get(arrayList, index);
        TEST_ASSERT_EQUAL_INT(listValues[index], value);
        index++;
    }

    ska_array_list_remove_by_index(arrayList, 1);
    int oneValue = (int)*(int*)ska_array_list_get(arrayList, 1);
    TEST_ASSERT_EQUAL_INT(3, oneValue);

    ska_array_list_remove(arrayList, &(int){3});
    oneValue = (int)*(int*)ska_array_list_get(arrayList, 1);
    TEST_ASSERT_EQUAL_INT(5, oneValue);

    ska_array_list_clear(arrayList);
    TEST_ASSERT_TRUE(ska_array_list_is_empty(arrayList));
    ska_array_list_destroy(arrayList);
}

void seika_id_queue_test(void) {
    SkaIdQueue* idQueue = ska_id_queue_create(10);

    uint32 value;
    for (uint32 i = 0; i < 10; i++) {
        ska_id_queue_dequeue(idQueue, &value);
        TEST_ASSERT_EQUAL_UINT32(i, value);
    }

    TEST_ASSERT_TRUE(ska_id_queue_is_empty(idQueue));

    //
    ska_id_queue_enqueue(idQueue, 4);
    ska_id_queue_enqueue(idQueue, 6);
    ska_id_queue_enqueue(idQueue, 5);
    ska_id_queue_dequeue(idQueue, &value);
    TEST_ASSERT_EQUAL_UINT32(4, value);
    ska_id_queue_dequeue(idQueue, &value);
    TEST_ASSERT_EQUAL_UINT32(6, value);
    ska_id_queue_dequeue(idQueue, &value);
    TEST_ASSERT_EQUAL_UINT32(5, value);

    ska_id_queue_dequeue(idQueue, &value);
    TEST_ASSERT_EQUAL_UINT32(10, value);
    TEST_ASSERT_EQUAL_size_t(20, idQueue->capacity);
}

void seika_asset_file_loader_test(void) {
    ska_asset_file_loader_initialize();

    ska_asset_file_loader_set_read_mode(SkaAssetFileLoaderReadMode_ARCHIVE);
    const bool hasLoaded = ska_asset_file_loader_load_archive("test/resources/test.pck");
    TEST_ASSERT_TRUE(hasLoaded);

    // File exists in archive
    SkaArchiveFileAsset existingFileAsset = ska_asset_file_loader_get_asset("test.txt");
    TEST_ASSERT_TRUE(ska_asset_file_loader_is_asset_valid(&existingFileAsset));
    // File doesn't exist
    SkaArchiveFileAsset nonExistingFileAsset = ska_asset_file_loader_get_asset("test.png");
    TEST_ASSERT_FALSE(ska_asset_file_loader_is_asset_valid(&nonExistingFileAsset));
    // Test loading from disk
    SkaArchiveFileAsset diskAsset = ska_asset_file_loader_load_asset_from_disk("test/resources/test.pck");
    TEST_ASSERT_TRUE(ska_asset_file_loader_is_asset_valid(&diskAsset));

    ska_asset_file_loader_finalize();
}

#if SKA_ECS
typedef struct TestValueComponent {
    int32 value;
} TestValueComponent;

typedef struct TestTransformComponent {
    SkaTransform2D transform2D;
} TestTransformComponent;

// test ecs callbacks

static int32 entityRegisteredInTestCount = 0;
void test_ecs_callback_on_entity_registered(SkaECSSystem* system, SkaEntity entity){
    entityRegisteredInTestCount++;
}

void seika_ecs_test(void) {
    ska_ecs_initialize();

    SKA_ECS_REGISTER_COMPONENT(TestValueComponent);
    SKA_ECS_REGISTER_COMPONENT(TestTransformComponent);

    // Test getting component type info
    const SkaComponentTypeInfo* valueTypeInfo = SKA_ECS_COMPONENT_TYPE_INFO(TestValueComponent);
    TEST_ASSERT_NOT_NULL(valueTypeInfo);
    TEST_ASSERT_EQUAL_STRING("TestValueComponent", valueTypeInfo->name);
    TEST_ASSERT_EQUAL_INT(1 << 0, valueTypeInfo->type);
    TEST_ASSERT_EQUAL_UINT32(0, valueTypeInfo->index);
    TEST_ASSERT_EQUAL_size_t(sizeof(TestValueComponent), valueTypeInfo->size);
    const SkaComponentTypeInfo* transformTypeInfo = SKA_ECS_COMPONENT_TYPE_INFO(TestTransformComponent);
    TEST_ASSERT_NOT_NULL(transformTypeInfo);
    TEST_ASSERT_EQUAL_STRING("TestTransformComponent", transformTypeInfo->name);
    TEST_ASSERT_EQUAL_INT(1 << 1, transformTypeInfo->type);
    TEST_ASSERT_EQUAL_UINT32(1, transformTypeInfo->index);
    TEST_ASSERT_EQUAL_size_t(sizeof(TestTransformComponent), transformTypeInfo->size);

    // Test creating ecs system
    const SkaEntity testEntity = ska_ecs_entity_create();

    SkaECSSystem* testValueEcsSystem = SKA_ECS_SYSTEM_CREATE("test value system", TestValueComponent);
    testValueEcsSystem->on_entity_registered_func = test_ecs_callback_on_entity_registered;
    ska_ecs_system_register(testValueEcsSystem);

    // Test creating with template
#define VALUE_TRANSFORM_SYSTEM_TEMPLATE &(SkaECSSystemTemplate){ \
    .name = "test value transform system", \
    .on_ec_system_register = NULL, \
    .on_ec_system_destroy = NULL, \
    .on_entity_registered_func = test_ecs_callback_on_entity_registered, \
    .on_entity_start_func = NULL, \
    .on_entity_end_func = NULL, \
    .on_entity_unregistered_func = NULL, \
    .on_entity_entered_scene_func = NULL, \
    .render_func = NULL, \
    .pre_update_all_func = NULL, \
    .post_update_all_func = NULL, \
    .update_func = NULL, \
    .fixed_update_func = NULL, \
    .network_callback_func = NULL \
}
    SKA_ECS_SYSTEM_REGISTER_FROM_TEMPLATE(VALUE_TRANSFORM_SYSTEM_TEMPLATE, TestValueComponent, TestTransformComponent);
#undef VALUE_TRANSFORM_SYSTEM_TEMPLATE

    // Test entity id enqueue and dequeue
#define TEST_ENTITY_QUEUE_AMOUNT 1000
    for (SkaEntity entity = 1; entity < TEST_ENTITY_QUEUE_AMOUNT; entity++) {
        const SkaEntity newEntity = ska_ecs_entity_create();
        TEST_ASSERT_EQUAL_UINT32(entity, newEntity);
    }
    TEST_ASSERT_EQUAL_size_t(TEST_ENTITY_QUEUE_AMOUNT, ska_ecs_entity_get_active_count());
    for (SkaEntity entity = 1; entity < TEST_ENTITY_QUEUE_AMOUNT; entity++) {
        ska_ecs_entity_return(entity);
    }
    TEST_ASSERT_EQUAL_size_t(1, ska_ecs_entity_get_active_count());
#undef TEST_ENTITY_QUEUE_AMOUNT

    // Test getting component
    TestValueComponent testComponent = { .value = 10 };
    ska_ecs_component_manager_set_component(testEntity, valueTypeInfo->index, &testComponent);
    TestValueComponent* returnedValueComponent = (TestValueComponent*)ska_ecs_component_manager_get_component(testEntity, valueTypeInfo->index);
    TEST_ASSERT_NOT_NULL(returnedValueComponent);
    TEST_ASSERT_EQUAL_INT(10, returnedValueComponent->value);
    TestTransformComponent transformComponent = { .transform2D =  { .position = { .x = 10.0f, .y = 20.0f}, .scale = SKA_VECTOR2_ONE, .rotation = 0.0f } };
    ska_ecs_component_manager_set_component(testEntity, transformTypeInfo->index, &transformComponent);
    TestTransformComponent* returnedTransformComponent = (TestTransformComponent*)ska_ecs_component_manager_get_component(testEntity, transformTypeInfo->index);
    TEST_ASSERT_NOT_NULL(returnedTransformComponent);
    TEST_ASSERT_EQUAL_FLOAT(10.0f, returnedTransformComponent->transform2D.position.x);
    TEST_ASSERT_EQUAL_FLOAT(20.0f, returnedTransformComponent->transform2D.position.y);

    // Test component events
    ska_ecs_system_update_entity_signature_with_systems(testEntity);
    TEST_ASSERT_EQUAL_INT(2, entityRegisteredInTestCount);

    ska_ecs_finalize();
}
#endif

#if SKA_INPUT

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

    const char* backspaceKeyName = ska_input_key_to_string(SkaInputKey_KEYBOARD_BACKSPACE);
    const SkaInputKey backSpaceKey = ska_input_string_to_key(backspaceKeyName);
    TEST_ASSERT_EQUAL_STRING("Backspace", backspaceKeyName);
    TEST_ASSERT_EQUAL_INT(SkaInputKey_KEYBOARD_BACKSPACE, backSpaceKey);

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

#endif // if SKA_INPUT
