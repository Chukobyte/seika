#include <unity.h>
#include <seika/ecs/ecs.h>
#include <seika/defines.h>

typedef struct TestComponentA {
    i32 value;
} TestComponentA;

typedef struct TestComponentB {
    f32 x;
    f32 y;
} TestComponentB;

void setUp(void) {}
void tearDown(void) {}

void seika_ecs_component_register_type_test(void);
void seika_ecs_component_set_get_has_remove_test(void);
void seika_ecs_component_signature_independence_test(void);
void seika_ecs_component_multi_entity_test(void);

i32 main(i32 argv, char** args) {
    UNITY_BEGIN();
    RUN_TEST(seika_ecs_component_register_type_test);
    RUN_TEST(seika_ecs_component_set_get_has_remove_test);
    RUN_TEST(seika_ecs_component_signature_independence_test);
    RUN_TEST(seika_ecs_component_multi_entity_test);
    return UNITY_END();
}

void seika_ecs_component_register_type_test(void) {
    ska_ecs_initialize();

    const SkaComponentTypeInfo* type_info = SKA_ECS_REGISTER_COMPONENT(TestComponentA);
    TEST_ASSERT_NOT_NULL(type_info);
    TEST_ASSERT_EQUAL_STRING("TestComponentA", type_info->name);
    TEST_ASSERT_EQUAL_UINT(sizeof(TestComponentA), type_info->size);
    TEST_ASSERT_EQUAL_UINT32(0, type_info->index);
    TEST_ASSERT_EQUAL_UINT32(1u << 0, type_info->type);

    // Registering the same type again should return the same info, not a new index
    const SkaComponentTypeInfo* type_info_again = SKA_ECS_REGISTER_COMPONENT(TestComponentA);
    TEST_ASSERT_EQUAL_UINT32(type_info->index, type_info_again->index);

    ska_ecs_finalize();
}

void seika_ecs_component_set_get_has_remove_test(void) {
    ska_ecs_initialize();

    const SkaComponentTypeInfo* type_info_a = SKA_ECS_REGISTER_COMPONENT(TestComponentA);
    const SkaEntity entity = ska_ecs_entity_create();
    ska_ecs_component_manager_reserve(entity);

    TEST_ASSERT_FALSE(ska_ecs_component_manager_has_component(entity, type_info_a->index));

    TestComponentA component_a = {.value = 42};
    ska_ecs_component_manager_set_component(entity, type_info_a->index, &component_a);

    TEST_ASSERT_TRUE(ska_ecs_component_manager_has_component(entity, type_info_a->index));
    TestComponentA* fetched = (TestComponentA*)ska_ecs_component_manager_get_component(entity, type_info_a->index);
    TEST_ASSERT_NOT_NULL(fetched);
    TEST_ASSERT_EQUAL_INT32(42, fetched->value);

    // Caller's local should be unaffected by later manager-side mutation (proves copy, not pointer, semantics)
    component_a.value = 999;
    fetched = (TestComponentA*)ska_ecs_component_manager_get_component(entity, type_info_a->index);
    TEST_ASSERT_EQUAL_INT32(42, fetched->value);

    ska_ecs_component_manager_remove_component(entity, type_info_a->index);
    TEST_ASSERT_FALSE(ska_ecs_component_manager_has_component(entity, type_info_a->index));

    ska_ecs_finalize();
}

void seika_ecs_component_signature_independence_test(void) {
    ska_ecs_initialize();

    const SkaComponentTypeInfo* type_info_a = SKA_ECS_REGISTER_COMPONENT(TestComponentA);
    const SkaComponentTypeInfo* type_info_b = SKA_ECS_REGISTER_COMPONENT(TestComponentB);
    const SkaEntity entity = ska_ecs_entity_create();

    TestComponentA component_a = {.value = 7};
    TestComponentB component_b = {.x = 1.0f, .y = 2.0f};
    ska_ecs_component_manager_set_component(entity, type_info_a->index, &component_a);
    ska_ecs_component_manager_set_component(entity, type_info_b->index, &component_b);

    TEST_ASSERT_TRUE(ska_ecs_component_manager_has_component(entity, type_info_a->index));
    TEST_ASSERT_TRUE(ska_ecs_component_manager_has_component(entity, type_info_b->index));

    // Removing A must not clobber B's signature bit (regression check for the '&=' vs '&= ~' bug)
    ska_ecs_component_manager_remove_component(entity, type_info_a->index);
    TEST_ASSERT_FALSE(ska_ecs_component_manager_has_component(entity, type_info_a->index));
    TEST_ASSERT_TRUE(ska_ecs_component_manager_has_component(entity, type_info_b->index));

    TestComponentB* fetched_b = (TestComponentB*)ska_ecs_component_manager_get_component(entity, type_info_b->index);
    TEST_ASSERT_EQUAL_FLOAT(1.0f, fetched_b->x);
    TEST_ASSERT_EQUAL_FLOAT(2.0f, fetched_b->y);

    ska_ecs_component_manager_remove_all_components(entity);
    TEST_ASSERT_FALSE(ska_ecs_component_manager_has_component(entity, type_info_a->index));
    TEST_ASSERT_FALSE(ska_ecs_component_manager_has_component(entity, type_info_b->index));

    ska_ecs_finalize();
}

void seika_ecs_component_multi_entity_test(void) {
    ska_ecs_initialize();

    const SkaComponentTypeInfo* type_info_a = SKA_ECS_REGISTER_COMPONENT(TestComponentA);

    // Set the component on a sparse set of entities to exercise pool growth/indexing
    SkaEntity entities[5];
    for (i32 i = 0; i < 5; i++) {
        entities[i] = ska_ecs_entity_create();
    }

    ska_ecs_component_manager_set_component(entities[0], type_info_a->index, &(TestComponentA){.value = 100});
    ska_ecs_component_manager_set_component(entities[2], type_info_a->index, &(TestComponentA){.value = 300});
    ska_ecs_component_manager_set_component(entities[4], type_info_a->index, &(TestComponentA){.value = 500});

    TEST_ASSERT_TRUE(ska_ecs_component_manager_has_component(entities[0], type_info_a->index));
    TEST_ASSERT_FALSE(ska_ecs_component_manager_has_component(entities[1], type_info_a->index));
    TEST_ASSERT_TRUE(ska_ecs_component_manager_has_component(entities[2], type_info_a->index));
    TEST_ASSERT_FALSE(ska_ecs_component_manager_has_component(entities[3], type_info_a->index));
    TEST_ASSERT_TRUE(ska_ecs_component_manager_has_component(entities[4], type_info_a->index));

    TEST_ASSERT_EQUAL_INT32(100, ((TestComponentA*)ska_ecs_component_manager_get_component(entities[0], type_info_a->index))->value);
    TEST_ASSERT_EQUAL_INT32(300, ((TestComponentA*)ska_ecs_component_manager_get_component(entities[2], type_info_a->index))->value);
    TEST_ASSERT_EQUAL_INT32(500, ((TestComponentA*)ska_ecs_component_manager_get_component(entities[4], type_info_a->index))->value);

    ska_ecs_finalize();
}
