#include <unity.h>
#include <seika/data_structures/array_list_template.h>
#include <seika/memory.h>
#include <string.h>
#include <math.h>

void setUp(void) {}
void tearDown(void) {}

// ===== TEST TYPES =====

typedef struct TestObject {
    i32 id;
    i32 refCount;
    char name[64];
} TestObject;

typedef struct ComplexStruct {
    i32 priority;
    f32 value;
    char tag[32];
} ComplexStruct;

// Declare and implement templated types
SKA_ARRAY_LIST_DECLARE(I32ArrayList, i32)
SKA_ARRAY_LIST_IMPLEMENT(I32ArrayList, i32)

SKA_ARRAY_LIST_DECLARE(ObjPtrArrayList, TestObject*)
SKA_ARRAY_LIST_IMPLEMENT(ObjPtrArrayList, TestObject*)

SKA_ARRAY_LIST_DECLARE(ObjPtrPtrArrayList, TestObject**)
SKA_ARRAY_LIST_IMPLEMENT(ObjPtrPtrArrayList, TestObject**)

SKA_ARRAY_LIST_DECLARE(ComplexArrayList, ComplexStruct)
SKA_ARRAY_LIST_IMPLEMENT(ComplexArrayList, ComplexStruct)

SKA_ARRAY_LIST_DECLARE(F32ArrayList, f32)
SKA_ARRAY_LIST_IMPLEMENT(F32ArrayList, f32)

// ===== CUSTOM COMPARE FUNCTIONS =====

// Compare TestObject* by ID
static bool test_obj_compare_by_id(TestObject* const* a, TestObject* const* b) {
    return (*a)->id == (*b)->id;
}

// Compare TestObject* by name
static bool test_obj_compare_by_name(TestObject* const* a, TestObject* const* b) {
    return strcmp((*a)->name, (*b)->name) == 0;
}

// Compare ComplexStruct by priority
static bool complex_compare_by_priority(ComplexStruct const* a, ComplexStruct const* b) {
    return a->priority == b->priority;
}

// ===== BASIC INT TESTS =====

void seika_array_list_template_init_test(void) {
    I32ArrayList list = I32ArrayList_init(10);

    TEST_ASSERT_NOT_NULL(list.data);
    TEST_ASSERT_EQUAL_UINT(10, list.capacity);
    TEST_ASSERT_EQUAL_UINT(0, list.size);

    I32ArrayList_deinit(&list);
}

void seika_array_list_template_init_default_capacity_test(void) {
    I32ArrayList list = I32ArrayList_init_default_capacity();

    TEST_ASSERT_NOT_NULL(list.data);
    TEST_ASSERT_EQUAL_UINT(16, list.capacity);
    TEST_ASSERT_EQUAL_UINT(0, list.size);

    I32ArrayList_deinit(&list);
}

void seika_array_list_template_append_test(void) {
    I32ArrayList list = I32ArrayList_init(5);

    TEST_ASSERT_TRUE(I32ArrayList_append(&list, 1));
    TEST_ASSERT_TRUE(I32ArrayList_append(&list, 2));
    TEST_ASSERT_TRUE(I32ArrayList_append(&list, 3));

    TEST_ASSERT_EQUAL_UINT(3, list.size);
    TEST_ASSERT_EQUAL_INT32(1, list.data[0]);
    TEST_ASSERT_EQUAL_INT32(2, list.data[1]);
    TEST_ASSERT_EQUAL_INT32(3, list.data[2]);

    I32ArrayList_deinit(&list);
}

void seika_array_list_template_get_test(void) {
    I32ArrayList list = I32ArrayList_init(5);

    I32ArrayList_append(&list, 10);
    I32ArrayList_append(&list, 20);
    I32ArrayList_append(&list, 30);

    i32* value = I32ArrayList_get(&list, 1);
    TEST_ASSERT_NOT_NULL(value);
    TEST_ASSERT_EQUAL_INT32(20, *value);

    I32ArrayList_deinit(&list);
}

void seika_array_list_template_get_const_test(void) {
    I32ArrayList list = I32ArrayList_init(5);

    I32ArrayList_append(&list, 10);
    I32ArrayList_append(&list, 20);

    const I32ArrayList* const_list = &list;
    i32 const* value = I32ArrayList_get_const(const_list, 0);

    TEST_ASSERT_NOT_NULL(value);
    TEST_ASSERT_EQUAL_INT32(10, *value);

    I32ArrayList_deinit(&list);
}

void seika_array_list_template_remove_test(void) {
    I32ArrayList list = I32ArrayList_init(5);

    I32ArrayList_append(&list, 10);
    I32ArrayList_append(&list, 20);
    I32ArrayList_append(&list, 30);

    TEST_ASSERT_TRUE(I32ArrayList_remove(&list, 20));
    TEST_ASSERT_EQUAL_UINT(2, list.size);
    TEST_ASSERT_EQUAL_INT32(10, list.data[0]);
    TEST_ASSERT_EQUAL_INT32(30, list.data[1]);

    TEST_ASSERT_FALSE(I32ArrayList_remove(&list, 999));

    I32ArrayList_deinit(&list);
}

void seika_array_list_template_remove_by_index_test(void) {
    I32ArrayList list = I32ArrayList_init(5);

    I32ArrayList_append(&list, 10);
    I32ArrayList_append(&list, 20);
    I32ArrayList_append(&list, 30);

    TEST_ASSERT_TRUE(I32ArrayList_remove_by_index(&list, 1));
    TEST_ASSERT_EQUAL_UINT(2, list.size);
    TEST_ASSERT_EQUAL_INT32(10, list.data[0]);
    TEST_ASSERT_EQUAL_INT32(30, list.data[1]);

    TEST_ASSERT_FALSE(I32ArrayList_remove_by_index(&list, 999));

    I32ArrayList_deinit(&list);
}

void seika_array_list_template_has_test(void) {
    I32ArrayList list = I32ArrayList_init(5);

    I32ArrayList_append(&list, 10);
    I32ArrayList_append(&list, 20);

    TEST_ASSERT_TRUE(I32ArrayList_has(&list, 10));
    TEST_ASSERT_TRUE(I32ArrayList_has(&list, 20));
    TEST_ASSERT_FALSE(I32ArrayList_has(&list, 999));

    I32ArrayList_deinit(&list);
}

void seika_array_list_template_is_empty_test(void) {
    I32ArrayList list = I32ArrayList_init(5);

    TEST_ASSERT_TRUE(I32ArrayList_is_empty(&list));

    I32ArrayList_append(&list, 10);
    TEST_ASSERT_FALSE(I32ArrayList_is_empty(&list));

    I32ArrayList_deinit(&list);
}

void seika_array_list_template_clear_test(void) {
    I32ArrayList list = I32ArrayList_init(5);

    I32ArrayList_append(&list, 10);
    I32ArrayList_append(&list, 20);
    I32ArrayList_append(&list, 30);

    I32ArrayList_clear(&list);

    TEST_ASSERT_EQUAL_UINT(0, list.size);
    TEST_ASSERT_TRUE(I32ArrayList_is_empty(&list));

    I32ArrayList_deinit(&list);
}

void seika_array_list_template_reserve_test(void) {
    I32ArrayList list = I32ArrayList_init(5);

    TEST_ASSERT_EQUAL_UINT(5, list.capacity);

    I32ArrayList_reserve(&list, 20);

    TEST_ASSERT_EQUAL_UINT(20, list.capacity);
    TEST_ASSERT_EQUAL_UINT(0, list.size);

    I32ArrayList_deinit(&list);
}

void seika_array_list_template_to_array_test(void) {
    I32ArrayList list = I32ArrayList_init(5);

    I32ArrayList_append(&list, 10);
    I32ArrayList_append(&list, 20);
    I32ArrayList_append(&list, 30);

    i32* array = I32ArrayList_to_array(&list);

    TEST_ASSERT_EQUAL_PTR(list.data, array);
    TEST_ASSERT_EQUAL_INT32(10, array[0]);
    TEST_ASSERT_EQUAL_INT32(20, array[1]);
    TEST_ASSERT_EQUAL_INT32(30, array[2]);

    I32ArrayList_deinit(&list);
}

// ===== DYNAMIC GROWTH TESTS =====

void seika_array_list_template_dynamic_growth_test(void) {
    I32ArrayList list = I32ArrayList_init(2);

    for (i32 i = 0; i < 100; i++) {
        I32ArrayList_append(&list, i);
    }

    TEST_ASSERT_EQUAL_UINT(100, list.size);
    TEST_ASSERT_GREATER_OR_EQUAL_UINT(100, list.capacity);

    for (i32 i = 0; i < 100; i++) {
        TEST_ASSERT_EQUAL_INT32(i, list.data[i]);
    }

    I32ArrayList_deinit(&list);
}

// ===== POINTER TESTS =====

void seika_array_list_template_pointer_basic_test(void) {
    ObjPtrArrayList list = ObjPtrArrayList_init(5);

    TestObject obj1 = {.id = 1, .refCount = 0};
    strcpy(obj1.name, "Object1");

    TestObject obj2 = {.id = 2, .refCount = 0};
    strcpy(obj2.name, "Object2");

    ObjPtrArrayList_append(&list, &obj1);
    ObjPtrArrayList_append(&list, &obj2);

    TEST_ASSERT_EQUAL_UINT(2, list.size);
    TEST_ASSERT_EQUAL_INT32(1, list.data[0]->id);
    TEST_ASSERT_EQUAL_INT32(2, list.data[1]->id);

    ObjPtrArrayList_deinit(&list);
}

void seika_array_list_template_pointer_remove_test(void) {
    ObjPtrArrayList list = ObjPtrArrayList_init(5);

    TestObject obj1 = {.id = 1, .refCount = 0};
    strcpy(obj1.name, "Object1");

    TestObject obj2 = {.id = 2, .refCount = 0};
    strcpy(obj2.name, "Object2");

    TestObject obj3 = {.id = 3, .refCount = 0};
    strcpy(obj3.name, "Object3");

    ObjPtrArrayList_append(&list, &obj1);
    ObjPtrArrayList_append(&list, &obj2);
    ObjPtrArrayList_append(&list, &obj3);

    TEST_ASSERT_TRUE(ObjPtrArrayList_remove2(&list, &obj2, test_obj_compare_by_id));
    TEST_ASSERT_EQUAL_UINT(2, list.size);
    TEST_ASSERT_EQUAL_INT32(1, list.data[0]->id);
    TEST_ASSERT_EQUAL_INT32(3, list.data[1]->id);

    ObjPtrArrayList_deinit(&list);
}

void seika_array_list_template_pointer_has_test(void) {
    ObjPtrArrayList list = ObjPtrArrayList_init(5);

    TestObject obj1 = {.id = 1, .refCount = 0};
    strcpy(obj1.name, "Object1");

    TestObject obj2 = {.id = 2, .refCount = 0};
    strcpy(obj2.name, "Object2");

    TestObject obj_not_in_list = {.id = 1, .refCount = 0};
    strcpy(obj_not_in_list.name, "NotInList");

    ObjPtrArrayList_append(&list, &obj1);
    ObjPtrArrayList_append(&list, &obj2);

    TEST_ASSERT_TRUE(ObjPtrArrayList_has2(&list, &obj1, test_obj_compare_by_id));
    TEST_ASSERT_TRUE(ObjPtrArrayList_has2(&list, &obj_not_in_list, test_obj_compare_by_id));
    TEST_ASSERT_FALSE(ObjPtrArrayList_has2(&list, &obj_not_in_list, test_obj_compare_by_name));

    ObjPtrArrayList_deinit(&list);
}

// ===== POINTER TO POINTER TESTS =====

void seika_array_list_template_ptr_ptr_test(void) {
    ObjPtrPtrArrayList list = ObjPtrPtrArrayList_init(5);

    TestObject obj1 = {.id = 100, .refCount = 0};
    TestObject* ptr1 = &obj1;

    TestObject obj2 = {.id = 200, .refCount = 0};
    TestObject* ptr2 = &obj2;

    ObjPtrPtrArrayList_append(&list, &ptr1);
    ObjPtrPtrArrayList_append(&list, &ptr2);

    TEST_ASSERT_EQUAL_UINT(2, list.size);
    TEST_ASSERT_EQUAL_INT32(100, (*list.data[0])->id);
    TEST_ASSERT_EQUAL_INT32(200, (*list.data[1])->id);

    ObjPtrPtrArrayList_deinit(&list);
}

// ===== STRUCT TESTS =====

void seika_array_list_template_struct_test(void) {
    ComplexArrayList list = ComplexArrayList_init(5);

    ComplexStruct s1 = {.priority = 1, .value = 10.5f};
    strcpy(s1.tag, "High");

    ComplexStruct s2 = {.priority = 2, .value = 20.3f};
    strcpy(s2.tag, "Medium");

    ComplexArrayList_append(&list, s1);
    ComplexArrayList_append(&list, s2);

    TEST_ASSERT_EQUAL_UINT(2, list.size);
    TEST_ASSERT_EQUAL_INT32(1, list.data[0].priority);
    TEST_ASSERT_EQUAL_FLOAT(10.5f, list.data[0].value);
    TEST_ASSERT_EQUAL_STRING("High", list.data[0].tag);

    ComplexArrayList_deinit(&list);
}

void seika_array_list_template_struct_custom_compare_test(void) {
    ComplexArrayList list = ComplexArrayList_init(5);

    ComplexStruct s1 = {.priority = 1, .value = 10.5f};
    ComplexStruct s2 = {.priority = 2, .value = 20.3f};
    ComplexStruct s3 = {.priority = 1, .value = 99.9f};

    ComplexArrayList_append(&list, s1);
    ComplexArrayList_append(&list, s2);
    ComplexArrayList_append(&list, s3);

    TEST_ASSERT_TRUE(ComplexArrayList_has2(&list, s1, complex_compare_by_priority));
    TEST_ASSERT_TRUE(ComplexArrayList_has2(&list, s3, complex_compare_by_priority));

    ComplexArrayList_deinit(&list);
}

// ===== FLOAT TESTS =====

void seika_array_list_template_float_test(void) {
    F32ArrayList list = F32ArrayList_init(5);

    F32ArrayList_append(&list, 1.5f);
    F32ArrayList_append(&list, 2.5f);
    F32ArrayList_append(&list, 3.5f);

    TEST_ASSERT_EQUAL_UINT(3, list.size);
    TEST_ASSERT_EQUAL_FLOAT(1.5f, list.data[0]);
    TEST_ASSERT_EQUAL_FLOAT(2.5f, list.data[1]);
    TEST_ASSERT_EQUAL_FLOAT(3.5f, list.data[2]);

    F32ArrayList_deinit(&list);
}

// ===== FOR-EACH MACRO TESTS =====

void seika_array_list_template_for_each_test(void) {
    I32ArrayList list = I32ArrayList_init(5);

    for (i32 i = 0; i < 5; i++) {
        I32ArrayList_append(&list, i * 10);
    }

    i32 sum = 0;
    SKA_ARRAY_LIST_T_FOR_EACH(&list, i32, iter) {
        sum += *iter;
    }

    TEST_ASSERT_EQUAL_INT32(100, sum);

    I32ArrayList_deinit(&list);
}

void seika_array_list_template_for_each_modify_test(void) {
    I32ArrayList list = I32ArrayList_init(5);

    for (i32 i = 0; i < 5; i++) {
        I32ArrayList_append(&list, i);
    }

    SKA_ARRAY_LIST_T_FOR_EACH(&list, i32, iter) {
        *iter *= 2;
    }

    for (i32 i = 0; i < 5; i++) {
        TEST_ASSERT_EQUAL_INT32(i * 2, list.data[i]);
    }

    I32ArrayList_deinit(&list);
}

void seika_array_list_template_for_each_empty_test(void) {
    I32ArrayList list = I32ArrayList_init(5);

    i32 count = 0;
    SKA_ARRAY_LIST_T_FOR_EACH(&list, i32, iter) {
        (void)iter;
        count++;
    }

    TEST_ASSERT_EQUAL_INT32(0, count);

    I32ArrayList_deinit(&list);
}

// ===== ALLOCATOR TEST =====

void seika_array_list_template_custom_allocator_test(void) {
    SkaAllocator custom_allocator = ska_mem_get_global_allocator();

    I32ArrayList list = I32ArrayList_init_a(custom_allocator, 10);

    I32ArrayList_append(&list, 42);

    TEST_ASSERT_EQUAL_UINT(1, list.size);
    TEST_ASSERT_EQUAL_INT32(42, list.data[0]);

    I32ArrayList_deinit(&list);
}

// ===== EDGE CASE TESTS =====

void seika_array_list_template_empty_operations_test(void) {
    I32ArrayList list = I32ArrayList_init(5);

    TEST_ASSERT_FALSE(I32ArrayList_remove(&list, 42));
    TEST_ASSERT_FALSE(I32ArrayList_has(&list, 42));
    TEST_ASSERT_TRUE(I32ArrayList_is_empty(&list));

    I32ArrayList_clear(&list);
    TEST_ASSERT_EQUAL_UINT(0, list.size);

    I32ArrayList_deinit(&list);
}

void seika_array_list_template_single_element_test(void) {
    I32ArrayList list = I32ArrayList_init(5);

    I32ArrayList_append(&list, 100);

    TEST_ASSERT_EQUAL_UINT(1, list.size);
    TEST_ASSERT_TRUE(I32ArrayList_has(&list, 100));

    i32* val = I32ArrayList_get(&list, 0);
    TEST_ASSERT_EQUAL_INT32(100, *val);

    TEST_ASSERT_TRUE(I32ArrayList_remove(&list, 100));
    TEST_ASSERT_EQUAL_UINT(0, list.size);

    I32ArrayList_deinit(&list);
}

void seika_array_list_template_duplicate_values_test(void) {
    I32ArrayList list = I32ArrayList_init(5);

    I32ArrayList_append(&list, 10);
    I32ArrayList_append(&list, 10);
    I32ArrayList_append(&list, 10);

    TEST_ASSERT_EQUAL_UINT(3, list.size);

    TEST_ASSERT_TRUE(I32ArrayList_remove(&list, 10));
    TEST_ASSERT_EQUAL_UINT(2, list.size);

    TEST_ASSERT_TRUE(I32ArrayList_has(&list, 10));

    I32ArrayList_deinit(&list);
}

// ===== MAIN TEST RUNNER =====

i32 main(i32 argv, char** args) {
    UNITY_BEGIN();

    // Basic int tests
    RUN_TEST(seika_array_list_template_init_test);
    RUN_TEST(seika_array_list_template_init_default_capacity_test);
    RUN_TEST(seika_array_list_template_append_test);
    RUN_TEST(seika_array_list_template_get_test);
    RUN_TEST(seika_array_list_template_get_const_test);
    RUN_TEST(seika_array_list_template_remove_test);
    RUN_TEST(seika_array_list_template_remove_by_index_test);
    RUN_TEST(seika_array_list_template_has_test);
    RUN_TEST(seika_array_list_template_is_empty_test);
    RUN_TEST(seika_array_list_template_clear_test);
    RUN_TEST(seika_array_list_template_reserve_test);
    RUN_TEST(seika_array_list_template_to_array_test);

    // Dynamic growth
    RUN_TEST(seika_array_list_template_dynamic_growth_test);

    // Pointer tests
    RUN_TEST(seika_array_list_template_pointer_basic_test);
    RUN_TEST(seika_array_list_template_pointer_remove_test);
    RUN_TEST(seika_array_list_template_pointer_has_test);

    // Pointer to pointer tests
    RUN_TEST(seika_array_list_template_ptr_ptr_test);

    // Struct tests
    RUN_TEST(seika_array_list_template_struct_test);
    RUN_TEST(seika_array_list_template_struct_custom_compare_test);

    // Float tests
    RUN_TEST(seika_array_list_template_float_test);

    // For-each macro tests
    RUN_TEST(seika_array_list_template_for_each_test);
    RUN_TEST(seika_array_list_template_for_each_modify_test);
    RUN_TEST(seika_array_list_template_for_each_empty_test);

    // Allocator test
    RUN_TEST(seika_array_list_template_custom_allocator_test);

    // Edge cases
    RUN_TEST(seika_array_list_template_empty_operations_test);
    RUN_TEST(seika_array_list_template_single_element_test);
    RUN_TEST(seika_array_list_template_duplicate_values_test);

    return UNITY_END();
}
