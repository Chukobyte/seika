#include <unity.h>
#include <string.h>

#include "seika/memory.h"
#include "seika/event.h"
#include "seika/input/input.h"
#include "seika/data_structures/hash_map.h"
#include "seika/data_structures/spatial_hash_map.h"
#include "seika/data_structures/array2d.h"
#include "seika/data_structures/linked_list.h"
#include "seika/ecs/ec_system.h"
#include "seika/ecs/ecs.h"
#include "seika/math/curve_float.h"
#include "seika/asset/asset_file_loader.h"
#include "seika/rendering/shader/shader_instance.h"
#include "seika/rendering/shader/shader_file_parser.h"

#define RESOURCES_PATH "test/resources"
#define RESOURCES_PACK_PATH "test/resources/test.pck"

void setUp(void) {}
void tearDown(void) {}

void seika_input_test(void);
void seika_hash_map_test(void);
void seika_spatial_hash_map_test(void);
void seika_linked_list_test(void);
void seika_array2d_test(void);

void seika_asset_file_loader_test(void);
void seika_observer_test(void);
void seika_curve_float_test(void);
void seika_shader_instance_test(void);
void seika_shader_file_parser_test(void);
void seika_ecs_test(void);

int32 main(int32 argv, char** args) {
    UNITY_BEGIN();
    RUN_TEST(seika_input_test);
    RUN_TEST(seika_hash_map_test);
    RUN_TEST(seika_spatial_hash_map_test);
    RUN_TEST(seika_linked_list_test);
    RUN_TEST(seika_array2d_test);
    RUN_TEST(seika_asset_file_loader_test);
    RUN_TEST(seika_observer_test);
    RUN_TEST(seika_curve_float_test);
    RUN_TEST(seika_shader_instance_test);
    RUN_TEST(seika_shader_file_parser_test);
    RUN_TEST(seika_ecs_test);
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

// Observer Test
static bool hasObserved = false;

void observer_func1(SkaSubjectNotifyPayload* payload) {
    hasObserved = true;
}

void observer_func2(SkaSubjectNotifyPayload* payload) {
    const int dataValue = *(int*) payload->data;
    if (dataValue == 3) {
        hasObserved = true;
    }
}

void seika_observer_test(void) {
    SkaEvent* event = ska_event_new();
    // Test 1 - Simple test with passing a NULL payload
    SkaObserver* observer = ska_observer_new(observer_func1);
    ska_event_register_observer(event, observer);
    TEST_ASSERT_EQUAL_INT(1, event->observerCount);
    ska_event_notify_observers(event, NULL);
    TEST_ASSERT(hasObserved);
    ska_event_unregister_observer(event, observer);
    TEST_ASSERT_EQUAL_INT(0, event->observerCount);
    hasObserved = false;

    // Test 2 - A slightly more complicated example filling out the payload
    ska_observer_delete(observer);
    observer = ska_observer_new(observer_func2);
    ska_event_register_observer(event, observer);
    int dataValue = 3;
    ska_event_notify_observers(event, &(SkaSubjectNotifyPayload) {
            .data = &dataValue
    });
    TEST_ASSERT(hasObserved);

    // Clean up
    ska_event_delete(event);
    ska_observer_delete(observer);
}

void seika_curve_float_test(void) {
    SkaCurveFloat curve = { .controlPointCount = 0 };
    SkaCurveControlPoint point1 = { .x = 0.0, .y = 0.0, .tangentIn = 0.0, .tangentOut = 0.0 };
    ska_curve_float_add_control_point(&curve, point1);
    TEST_ASSERT_EQUAL_UINT(1, curve.controlPointCount);
    TEST_ASSERT_EQUAL_DOUBLE(0.0, ska_curve_float_eval(&curve, 1.0));
    SkaCurveControlPoint point2 = { .x = 1.0, .y = 1.0, .tangentIn = 0.0, .tangentOut = 0.0 };
    ska_curve_float_add_control_point(&curve, point2);
    TEST_ASSERT_EQUAL_DOUBLE(0.0, ska_curve_float_eval(&curve, 0.0));
    TEST_ASSERT_EQUAL_DOUBLE(0.5, ska_curve_float_eval(&curve, 0.5));
    TEST_ASSERT_EQUAL_DOUBLE(1.0, ska_curve_float_eval(&curve, 1.0));
    ska_curve_float_remove_control_point(&curve, point2.x, point2.y);
    TEST_ASSERT_EQUAL_UINT(1, curve.controlPointCount);

    // TODO: Write performance tests
//    SE_PROFILE_CODE(
//        for (int32 i = 0; i < 10000000; i++) {}
//    )
//
//    f64 cpu_time_used;
//    SE_PROFILE_CODE_WITH_VAR(cpu_time_used, for (int32 i = 0; i < 10000000; i++) {})
//    printf("Time taken: %f seconds\n", cpu_time_used);
}

void seika_shader_instance_test(void) {
    // Shader instance param tests
    SkaShaderInstance shaderInstance = { .shader = NULL, .paramMap = ska_string_hash_map_create_default_capacity() };

    ska_shader_instance_param_create_bool(&shaderInstance, "is_active", false);
    TEST_ASSERT_FALSE(ska_shader_instance_param_get_bool(&shaderInstance, "is_active"));
    ska_shader_instance_param_update_bool(&shaderInstance, "is_active", true);
    TEST_ASSERT_TRUE(ska_shader_instance_param_get_bool(&shaderInstance, "is_active"));

    // Clean up
    SKA_STRING_HASH_MAP_FOR_EACH(shaderInstance.paramMap, iter) {
        SkaStringHashMapNode* node = iter.pair;
        SkaShaderParam* param = (SkaShaderParam*) node->value;
        SKA_MEM_FREE(param->name);
    }
    ska_string_hash_map_destroy(shaderInstance.paramMap);
}

void seika_shader_file_parser_test(void) {
    char shader[] =
            "shader_type screen;\n"
            "\n"
            "uniform float time;\n"
            "uniform vec2 size;\n"
            "uniform float brightness = 1.0f;\n"
            "uniform int spriteCount = 1;\n"
            "\n"
            "vec3 testFunc() {\n"
            "    return vec3(1.0f);\n"
            "}\n"
            "\n"
            "void vertex() {\n"
            "    VERTEX.x += 0.1f;"
            "}\n"
            "\n"
            "void fragment() {\n"
            "    COLOR *= brightness;\n"
            "}\n"
            "\n";
    SkaShaderFileParseResult result = ska_shader_file_parser_parse_shader(shader);
    // Shouldn't be an error message
    const bool hasErrorMessage = strlen(result.errorMessage) > 0;
    if (hasErrorMessage) {
        printf("Shader parse error = '%s'\n", result.errorMessage);
    }
    TEST_ASSERT_FALSE(hasErrorMessage);
    ska_shader_file_parse_clear_parse_result(&result);
}

// ECS TEST
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
