#include <unity.h>
#include <string.h>

#include "seika/memory.h"
#include "seika/data_structures/array_list.h"
#include "seika/data_structures/id_queue.h"
#include "seika/math/math.h"

#if SKA_ECS
#include "seika/ecs/ecs.h"
#include "seika/ecs/ec_system.h"
#endif

#define RESOURCES_PATH "test/resources"
#define RESOURCES_PACK_PATH "test/resources/test.pck"

void setUp(void) {}
void tearDown(void) {}

void seika_mem_test(void);
void seika_array_list_test(void);
void seika_id_queue_test(void);

#if SKA_ECS
void seika_ecs_test(void);
#endif

int32 main(int32 argv, char** args) {
    UNITY_BEGIN();
    RUN_TEST(seika_mem_test);
    RUN_TEST(seika_array_list_test);
    RUN_TEST(seika_id_queue_test);
#if SKA_ECS
    RUN_TEST(seika_ecs_test);
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
