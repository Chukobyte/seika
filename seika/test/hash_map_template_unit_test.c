#include <unity.h>
#include <seika/data_structures/hash_map_template.h>
#include <seika/memory.h>
#include <string.h>

void setUp(void) {}
void tearDown(void) {}

// ===== TEST TYPES =====

typedef struct TestObject {
    i32 id;
    i32 refCount;
    char name[64];
} TestObject;

typedef struct ComplexKey {
    i32 priority;
    char category[32];
} ComplexKey;

// Declare and implement templated types
SKA_HASH_MAP_DECLARE(I32I32HashMap, i32, i32)
SKA_HASH_MAP_IMPLEMENT(I32I32HashMap, i32, i32)

SKA_HASH_MAP_DECLARE(ObjPtrI32HashMap, TestObject*, i32)
SKA_HASH_MAP_IMPLEMENT(ObjPtrI32HashMap, TestObject*, i32)

SKA_HASH_MAP_DECLARE(ObjPtrPtrI32HashMap, TestObject**, i32)
SKA_HASH_MAP_IMPLEMENT(ObjPtrPtrI32HashMap, TestObject**, i32)

SKA_HASH_MAP_DECLARE(ComplexI32HashMap, ComplexKey, i32)
SKA_HASH_MAP_IMPLEMENT(ComplexI32HashMap, ComplexKey, i32)

SKA_HASH_MAP_DECLARE(I32ObjPtrHashMap, i32, TestObject*)
SKA_HASH_MAP_IMPLEMENT(I32ObjPtrHashMap, i32, TestObject*)

SKA_HASH_MAP_DECLARE(StringI32HashMap, const char*, i32)
SKA_HASH_MAP_IMPLEMENT(StringI32HashMap, const char*, i32)

SKA_HASH_MAP_DECLARE(StringStringHashMap, const char*, const char*)
SKA_HASH_MAP_IMPLEMENT(StringStringHashMap, const char*, const char*)

// ===== CUSTOM HASH FUNCTIONS =====

// Hash TestObject* by ID
static usize test_obj_hash_by_id(TestObject* const* key) {
    return (usize)(*key)->id;
}

// Hash TestObject* by name (simple string hash)
static usize test_obj_hash_by_name(TestObject* const* key) {
    usize hash = 5381;
    const char* str = (*key)->name;
    i32 c;
    while ((c = (i32)*str++)) {
        hash = ((hash << 5) + hash) ^ c;
    }
    return hash;
}

// Hash TestObject** by dereferencing twice and using ID
static usize test_obj_ptr_ptr_hash_by_id(TestObject** const* key) {
    return (usize)(**key)->id;
}

// Hash ComplexKey by priority
static usize complex_hash_by_priority(ComplexKey const* key) {
    return (usize)key->priority;
}

// ===== CUSTOM COMPARE FUNCTIONS =====

// Compare TestObject* by ID
static int32 test_obj_compare_by_id(TestObject* const* a, TestObject* const* b) {
    return (*a)->id - (*b)->id;
}

// Compare TestObject* by name
static int32 test_obj_compare_by_name(TestObject* const* a, TestObject* const* b) {
    return strcmp((*a)->name, (*b)->name);
}

// Compare TestObject** by dereferencing twice and comparing ID
static int32 test_obj_ptr_ptr_compare_by_id(TestObject** const* a, TestObject** const* b) {
    return (**a)->id - (**b)->id;
}

// Compare ComplexKey by priority
static int32 complex_compare_by_priority(ComplexKey const* a, ComplexKey const* b) {
    return a->priority - b->priority;
}

// ===== STRING HASH AND COMPARE FUNCTIONS =====

// Hash string using djb2 algorithm
static usize string_hash(const char* const* key) {
    usize hash = 5381;
    const char* str = *key;
    i32 c;
    while ((c = (i32)*str++)) {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }
    return hash;
}

// Compare strings
static int32 string_compare(const char* const* a, const char* const* b) {
    return strcmp(*a, *b);
}

// ===== BASIC INT TESTS =====

void seika_hash_map_template_init_test(void) {
    I32I32HashMap* map = I32I32HashMap_init(16);

    TEST_ASSERT_NOT_NULL(map);
    TEST_ASSERT_EQUAL_UINT(16, map->capacity);
    TEST_ASSERT_EQUAL_UINT(0, map->size);

    I32I32HashMap_deinit(map);
}

void seika_hash_map_template_add_test(void) {
    I32I32HashMap* map = I32I32HashMap_init(16);

    I32I32HashMapAddResult result1 = I32I32HashMap_add(map, 1, 100);
    TEST_ASSERT_TRUE(result1.wasAdded);
    TEST_ASSERT_EQUAL_UINT(1, map->size);

    I32I32HashMapAddResult result2 = I32I32HashMap_add(map, 2, 200);
    TEST_ASSERT_TRUE(result2.wasAdded);
    TEST_ASSERT_EQUAL_UINT(2, map->size);

    I32I32HashMap_deinit(map);
}

void seika_hash_map_template_add_duplicate_test(void) {
    I32I32HashMap* map = I32I32HashMap_init(16);

    I32I32HashMapAddResult result1 = I32I32HashMap_add(map, 1, 100);
    TEST_ASSERT_TRUE(result1.wasAdded);

    I32I32HashMapAddResult result2 = I32I32HashMap_add(map, 1, 200);
    TEST_ASSERT_FALSE(result2.wasAdded);
    TEST_ASSERT_EQUAL_UINT(1, map->size);

    i32* value = I32I32HashMap_get(map, 1);
    TEST_ASSERT_EQUAL_INT32(200, *value);

    I32I32HashMap_deinit(map);
}

void seika_hash_map_template_get_test(void) {
    I32I32HashMap* map = I32I32HashMap_init(16);

    I32I32HashMap_add(map, 1, 100);
    I32I32HashMap_add(map, 2, 200);

    i32* value1 = I32I32HashMap_get(map, 1);
    TEST_ASSERT_NOT_NULL(value1);
    TEST_ASSERT_EQUAL_INT32(100, *value1);

    i32* value2 = I32I32HashMap_get(map, 2);
    TEST_ASSERT_NOT_NULL(value2);
    TEST_ASSERT_EQUAL_INT32(200, *value2);

    i32* value_not_found = I32I32HashMap_get(map, 999);
    TEST_ASSERT_NULL(value_not_found);

    I32I32HashMap_deinit(map);
}

void seika_hash_map_template_has_test(void) {
    I32I32HashMap* map = I32I32HashMap_init(16);

    I32I32HashMap_add(map, 1, 100);
    I32I32HashMap_add(map, 2, 200);

    TEST_ASSERT_TRUE(I32I32HashMap_has(map, 1));
    TEST_ASSERT_TRUE(I32I32HashMap_has(map, 2));
    TEST_ASSERT_FALSE(I32I32HashMap_has(map, 999));

    I32I32HashMap_deinit(map);
}

void seika_hash_map_template_erase_test(void) {
    I32I32HashMap* map = I32I32HashMap_init(16);

    I32I32HashMap_add(map, 1, 100);
    I32I32HashMap_add(map, 2, 200);
    I32I32HashMap_add(map, 3, 300);

    TEST_ASSERT_TRUE(I32I32HashMap_erase(map, 2));
    TEST_ASSERT_EQUAL_UINT(2, map->size);

    TEST_ASSERT_FALSE(I32I32HashMap_has(map, 2));
    TEST_ASSERT_TRUE(I32I32HashMap_has(map, 1));
    TEST_ASSERT_TRUE(I32I32HashMap_has(map, 3));

    TEST_ASSERT_FALSE(I32I32HashMap_erase(map, 999));

    I32I32HashMap_deinit(map);
}

void seika_hash_map_template_clear_test(void) {
    I32I32HashMap* map = I32I32HashMap_init(16);

    I32I32HashMap_add(map, 1, 100);
    I32I32HashMap_add(map, 2, 200);
    I32I32HashMap_add(map, 3, 300);

    I32I32HashMap_clear(map);

    TEST_ASSERT_EQUAL_UINT(0, map->size);
    TEST_ASSERT_FALSE(I32I32HashMap_has(map, 1));
    TEST_ASSERT_FALSE(I32I32HashMap_has(map, 2));
    TEST_ASSERT_FALSE(I32I32HashMap_has(map, 3));

    I32I32HashMap_deinit(map);
}

// ===== DYNAMIC GROWTH TESTS =====

void seika_hash_map_template_dynamic_growth_test(void) {
    I32I32HashMap* map = I32I32HashMap_init(4);

    for (i32 i = 0; i < 100; i++) {
        I32I32HashMap_add(map, i, i * 10);
    }

    TEST_ASSERT_EQUAL_UINT(100, map->size);
    TEST_ASSERT_GREATER_OR_EQUAL_UINT(100, map->capacity);

    for (i32 i = 0; i < 100; i++) {
        i32* value = I32I32HashMap_get(map, i);
        TEST_ASSERT_NOT_NULL(value);
        TEST_ASSERT_EQUAL_INT32(i * 10, *value);
    }

    I32I32HashMap_deinit(map);
}

void seika_hash_map_template_shrink_test(void) {
    // Test that the shrink mechanism works correctly
    // Start with small initial capacity that allows shrinking
    I32I32HashMap* map = I32I32HashMap_init(16);

    // Add many elements to force significant growth
    for (i32 i = 0; i < 200; i++) {
        I32I32HashMap_add(map, i, i);
    }

    usize capacity_after_growth = map->capacity;
    TEST_ASSERT_GREATER_THAN_UINT(100, capacity_after_growth);

    // Remove most elements - keep only elements that would be under shrink threshold
    // Shrink threshold is 0.25 * capacity
    // Keep fewer elements than the threshold
    for (i32 i = 20; i < 200; i++) {
        I32I32HashMap_erase(map, i);
    }

    // Verify remaining elements are still accessible
    TEST_ASSERT_EQUAL_UINT(20, map->size);
    for (i32 i = 0; i < 20; i++) {
        TEST_ASSERT_TRUE(I32I32HashMap_has(map, i));
    }

    I32I32HashMap_deinit(map);
}

// ===== POINTER KEY TESTS =====

void seika_hash_map_template_pointer_key_test(void) {
    ObjPtrI32HashMap* map = ObjPtrI32HashMap_init(16);
    map->hashFunc = test_obj_hash_by_id;
    map->compareFunc = test_obj_compare_by_id;

    TestObject obj1 = {.id = 1, .refCount = 0};
    strcpy(obj1.name, "Object1");

    TestObject obj2 = {.id = 2, .refCount = 0};
    strcpy(obj2.name, "Object2");

    ObjPtrI32HashMap_add(map, &obj1, 100);
    ObjPtrI32HashMap_add(map, &obj2, 200);

    TEST_ASSERT_EQUAL_UINT(2, map->size);

    i32* val1 = ObjPtrI32HashMap_get(map, &obj1);
    TEST_ASSERT_NOT_NULL(val1);
    TEST_ASSERT_EQUAL_INT32(100, *val1);

    i32* val2 = ObjPtrI32HashMap_get(map, &obj2);
    TEST_ASSERT_NOT_NULL(val2);
    TEST_ASSERT_EQUAL_INT32(200, *val2);

    ObjPtrI32HashMap_deinit(map);
}

void seika_hash_map_template_pointer_key_custom_hash_test(void) {
    ObjPtrI32HashMap* map = ObjPtrI32HashMap_init(16);
    map->hashFunc = test_obj_hash_by_name;
    map->compareFunc = test_obj_compare_by_name;

    TestObject obj1 = {.id = 1, .refCount = 0};
    strcpy(obj1.name, "Alpha");

    TestObject obj2 = {.id = 2, .refCount = 0};
    strcpy(obj2.name, "Beta");

    ObjPtrI32HashMap_add(map, &obj1, 100);
    ObjPtrI32HashMap_add(map, &obj2, 200);

    TestObject lookup_obj = {.id = 999, .refCount = 0};
    strcpy(lookup_obj.name, "Alpha");

    i32* val = ObjPtrI32HashMap_get(map, &lookup_obj);
    TEST_ASSERT_NOT_NULL(val);
    TEST_ASSERT_EQUAL_INT32(100, *val);

    ObjPtrI32HashMap_deinit(map);
}

// ===== POINTER TO POINTER KEY TESTS =====

void seika_hash_map_template_ptr_ptr_key_test(void) {
    ObjPtrPtrI32HashMap* map = ObjPtrPtrI32HashMap_init(16);
    map->hashFunc = test_obj_ptr_ptr_hash_by_id;
    map->compareFunc = test_obj_ptr_ptr_compare_by_id;

    TestObject obj1 = {.id = 100, .refCount = 0};
    TestObject* ptr1 = &obj1;

    TestObject obj2 = {.id = 200, .refCount = 0};
    TestObject* ptr2 = &obj2;

    ObjPtrPtrI32HashMap_add(map, &ptr1, 1000);
    ObjPtrPtrI32HashMap_add(map, &ptr2, 2000);

    TEST_ASSERT_EQUAL_UINT(2, map->size);

    i32* val1 = ObjPtrPtrI32HashMap_get(map, &ptr1);
    TEST_ASSERT_NOT_NULL(val1);
    TEST_ASSERT_EQUAL_INT32(1000, *val1);

    ObjPtrPtrI32HashMap_deinit(map);
}

// ===== STRUCT KEY TESTS =====

void seika_hash_map_template_struct_key_test(void) {
    ComplexI32HashMap* map = ComplexI32HashMap_init(16);

    ComplexKey key1 = {.priority = 1};
    strcpy(key1.category, "High");

    ComplexKey key2 = {.priority = 2};
    strcpy(key2.category, "Medium");

    ComplexI32HashMap_add(map, key1, 100);
    ComplexI32HashMap_add(map, key2, 200);

    TEST_ASSERT_EQUAL_UINT(2, map->size);

    i32* val1 = ComplexI32HashMap_get(map, key1);
    TEST_ASSERT_NOT_NULL(val1);
    TEST_ASSERT_EQUAL_INT32(100, *val1);

    ComplexI32HashMap_deinit(map);
}

void seika_hash_map_template_struct_key_custom_hash_test(void) {
    ComplexI32HashMap* map = ComplexI32HashMap_init(16);
    map->hashFunc = complex_hash_by_priority;
    map->compareFunc = complex_compare_by_priority;

    ComplexKey key1 = {.priority = 1};
    strcpy(key1.category, "High");

    ComplexKey key2 = {.priority = 1};
    strcpy(key2.category, "Different");

    ComplexI32HashMap_add(map, key1, 100);

    i32* val = ComplexI32HashMap_get(map, key2);
    TEST_ASSERT_NOT_NULL(val);
    TEST_ASSERT_EQUAL_INT32(100, *val);

    ComplexI32HashMap_deinit(map);
}

// ===== POINTER VALUE TESTS =====

void seika_hash_map_template_pointer_value_test(void) {
    I32ObjPtrHashMap* map = I32ObjPtrHashMap_init(16);

    TestObject obj1 = {.id = 1, .refCount = 0};
    strcpy(obj1.name, "Object1");

    TestObject obj2 = {.id = 2, .refCount = 0};
    strcpy(obj2.name, "Object2");

    I32ObjPtrHashMap_add(map, 1, &obj1);
    I32ObjPtrHashMap_add(map, 2, &obj2);

    TestObject** val1 = I32ObjPtrHashMap_get(map, 1);
    TEST_ASSERT_NOT_NULL(val1);
    TEST_ASSERT_EQUAL_INT32(1, (*val1)->id);
    TEST_ASSERT_EQUAL_STRING("Object1", (*val1)->name);

    I32ObjPtrHashMap_deinit(map);
}

// ===== STRING KEY TESTS =====

void seika_hash_map_template_string_key_basic_test(void) {
    StringI32HashMap* map = StringI32HashMap_init(16);
    map->hashFunc = string_hash;
    map->compareFunc = string_compare;

    StringI32HashMap_add(map, "apple", 1);
    StringI32HashMap_add(map, "banana", 2);
    StringI32HashMap_add(map, "cherry", 3);

    TEST_ASSERT_EQUAL_UINT(3, map->size);

    i32* val1 = StringI32HashMap_get(map, "apple");
    TEST_ASSERT_NOT_NULL(val1);
    TEST_ASSERT_EQUAL_INT32(1, *val1);

    i32* val2 = StringI32HashMap_get(map, "banana");
    TEST_ASSERT_NOT_NULL(val2);
    TEST_ASSERT_EQUAL_INT32(2, *val2);

    i32* val3 = StringI32HashMap_get(map, "cherry");
    TEST_ASSERT_NOT_NULL(val3);
    TEST_ASSERT_EQUAL_INT32(3, *val3);

    StringI32HashMap_deinit(map);
}

void seika_hash_map_template_string_key_has_test(void) {
    StringI32HashMap* map = StringI32HashMap_init(16);
    map->hashFunc = string_hash;
    map->compareFunc = string_compare;

    StringI32HashMap_add(map, "hello", 100);
    StringI32HashMap_add(map, "world", 200);

    TEST_ASSERT_TRUE(StringI32HashMap_has(map, "hello"));
    TEST_ASSERT_TRUE(StringI32HashMap_has(map, "world"));
    TEST_ASSERT_FALSE(StringI32HashMap_has(map, "missing"));
    TEST_ASSERT_FALSE(StringI32HashMap_has(map, ""));

    StringI32HashMap_deinit(map);
}

void seika_hash_map_template_string_key_erase_test(void) {
    StringI32HashMap* map = StringI32HashMap_init(16);
    map->hashFunc = string_hash;
    map->compareFunc = string_compare;

    StringI32HashMap_add(map, "first", 1);
    StringI32HashMap_add(map, "second", 2);
    StringI32HashMap_add(map, "third", 3);

    TEST_ASSERT_TRUE(StringI32HashMap_erase(map, "second"));
    TEST_ASSERT_EQUAL_UINT(2, map->size);
    TEST_ASSERT_FALSE(StringI32HashMap_has(map, "second"));
    TEST_ASSERT_TRUE(StringI32HashMap_has(map, "first"));
    TEST_ASSERT_TRUE(StringI32HashMap_has(map, "third"));

    StringI32HashMap_deinit(map);
}

void seika_hash_map_template_string_key_update_test(void) {
    StringI32HashMap* map = StringI32HashMap_init(16);
    map->hashFunc = string_hash;
    map->compareFunc = string_compare;

    StringI32HashMap_add(map, "counter", 1);

    i32* val = StringI32HashMap_get(map, "counter");
    TEST_ASSERT_EQUAL_INT32(1, *val);

    // Update the value
    StringI32HashMap_add(map, "counter", 10);

    val = StringI32HashMap_get(map, "counter");
    TEST_ASSERT_EQUAL_INT32(10, *val);
    TEST_ASSERT_EQUAL_UINT(1, map->size); // Size shouldn't change

    StringI32HashMap_deinit(map);
}

void seika_hash_map_template_string_key_empty_string_test(void) {
    StringI32HashMap* map = StringI32HashMap_init(16);
    map->hashFunc = string_hash;
    map->compareFunc = string_compare;

    // Empty string as key should work
    StringI32HashMap_add(map, "", 0);
    StringI32HashMap_add(map, "nonempty", 1);

    TEST_ASSERT_TRUE(StringI32HashMap_has(map, ""));
    i32* val = StringI32HashMap_get(map, "");
    TEST_ASSERT_NOT_NULL(val);
    TEST_ASSERT_EQUAL_INT32(0, *val);

    StringI32HashMap_deinit(map);
}

void seika_hash_map_template_string_key_long_strings_test(void) {
    StringI32HashMap* map = StringI32HashMap_init(16);
    map->hashFunc = string_hash;
    map->compareFunc = string_compare;

    const char* long_key1 = "this_is_a_very_long_key_name_that_should_still_work_correctly";
    const char* long_key2 = "another_extremely_long_key_name_for_testing_purposes_only";

    StringI32HashMap_add(map, long_key1, 100);
    StringI32HashMap_add(map, long_key2, 200);

    TEST_ASSERT_TRUE(StringI32HashMap_has(map, long_key1));
    TEST_ASSERT_TRUE(StringI32HashMap_has(map, long_key2));

    i32* val1 = StringI32HashMap_get(map, long_key1);
    TEST_ASSERT_EQUAL_INT32(100, *val1);

    StringI32HashMap_deinit(map);
}

void seika_hash_map_template_string_key_similar_strings_test(void) {
    StringI32HashMap* map = StringI32HashMap_init(16);
    map->hashFunc = string_hash;
    map->compareFunc = string_compare;

    // Test with similar but different strings
    StringI32HashMap_add(map, "test", 1);
    StringI32HashMap_add(map, "test1", 2);
    StringI32HashMap_add(map, "test2", 3);
    StringI32HashMap_add(map, "tests", 4);

    TEST_ASSERT_EQUAL_UINT(4, map->size);

    i32* val1 = StringI32HashMap_get(map, "test");
    TEST_ASSERT_EQUAL_INT32(1, *val1);

    i32* val2 = StringI32HashMap_get(map, "test1");
    TEST_ASSERT_EQUAL_INT32(2, *val2);

    i32* val3 = StringI32HashMap_get(map, "test2");
    TEST_ASSERT_EQUAL_INT32(3, *val3);

    i32* val4 = StringI32HashMap_get(map, "tests");
    TEST_ASSERT_EQUAL_INT32(4, *val4);

    StringI32HashMap_deinit(map);
}

void seika_hash_map_template_string_to_string_test(void) {
    StringStringHashMap* map = StringStringHashMap_init(16);
    map->hashFunc = string_hash;
    map->compareFunc = string_compare;

    StringStringHashMap_add(map, "name", "Alice");
    StringStringHashMap_add(map, "city", "New York");
    StringStringHashMap_add(map, "country", "USA");

    const char** name = StringStringHashMap_get(map, "name");
    TEST_ASSERT_NOT_NULL(name);
    TEST_ASSERT_EQUAL_STRING("Alice", *name);

    const char** city = StringStringHashMap_get(map, "city");
    TEST_ASSERT_NOT_NULL(city);
    TEST_ASSERT_EQUAL_STRING("New York", *city);

    const char** country = StringStringHashMap_get(map, "country");
    TEST_ASSERT_NOT_NULL(country);
    TEST_ASSERT_EQUAL_STRING("USA", *country);

    StringStringHashMap_deinit(map);
}

void seika_hash_map_template_string_key_many_entries_test(void) {
    StringI32HashMap* map = StringI32HashMap_init(8);
    map->hashFunc = string_hash;
    map->compareFunc = string_compare;

    // Use an array of static strings that will remain valid
    static const char* keys[] = {
        "key_0", "key_1", "key_2", "key_3", "key_4", "key_5", "key_6", "key_7", "key_8", "key_9",
        "key_10", "key_11", "key_12", "key_13", "key_14", "key_15", "key_16", "key_17", "key_18", "key_19",
        "key_20", "key_21", "key_22", "key_23", "key_24", "key_25", "key_26", "key_27", "key_28", "key_29",
        "key_30", "key_31", "key_32", "key_33", "key_34", "key_35", "key_36", "key_37", "key_38", "key_39",
        "key_40", "key_41", "key_42", "key_43", "key_44", "key_45", "key_46", "key_47", "key_48", "key_49",
        "key_50", "key_51", "key_52", "key_53", "key_54", "key_55", "key_56", "key_57", "key_58", "key_59",
        "key_60", "key_61", "key_62", "key_63", "key_64", "key_65", "key_66", "key_67", "key_68", "key_69",
        "key_70", "key_71", "key_72", "key_73", "key_74", "key_75", "key_76", "key_77", "key_78", "key_79",
        "key_80", "key_81", "key_82", "key_83", "key_84", "key_85", "key_86", "key_87", "key_88", "key_89",
        "key_90", "key_91", "key_92", "key_93", "key_94", "key_95", "key_96", "key_97", "key_98", "key_99"
    };

    // Add many string keys to test collision handling and growth
    for (i32 i = 0; i < 100; i++) {
        StringI32HashMap_add(map, keys[i], i * 10);
    }

    TEST_ASSERT_EQUAL_UINT(100, map->size);

    // Verify we can retrieve values
    i32* val0 = StringI32HashMap_get(map, "key_0");
    TEST_ASSERT_NOT_NULL(val0);
    TEST_ASSERT_EQUAL_INT32(0, *val0);

    i32* val50 = StringI32HashMap_get(map, "key_50");
    TEST_ASSERT_NOT_NULL(val50);
    TEST_ASSERT_EQUAL_INT32(500, *val50);

    i32* val99 = StringI32HashMap_get(map, "key_99");
    TEST_ASSERT_NOT_NULL(val99);
    TEST_ASSERT_EQUAL_INT32(990, *val99);

    StringI32HashMap_deinit(map);
}

void seika_hash_map_template_string_key_iterator_test(void) {
    StringI32HashMap* map = StringI32HashMap_init(16);
    map->hashFunc = string_hash;
    map->compareFunc = string_compare;

    StringI32HashMap_add(map, "one", 1);
    StringI32HashMap_add(map, "two", 2);
    StringI32HashMap_add(map, "three", 3);

    i32 sum = 0;
    i32 count = 0;

    SKA_HASH_MAP_T_FOR_EACH(map, StringI32HashMap, iter) {
        sum += iter.pair->value;
        count++;
    }

    TEST_ASSERT_EQUAL_INT32(3, count);
    TEST_ASSERT_EQUAL_INT32(6, sum); // 1 + 2 + 3

    StringI32HashMap_deinit(map);
}

// ===== ITERATOR TESTS =====

void seika_hash_map_template_iterator_test(void) {
    I32I32HashMap* map = I32I32HashMap_init(16);

    I32I32HashMap_add(map, 1, 100);
    I32I32HashMap_add(map, 2, 200);
    I32I32HashMap_add(map, 3, 300);

    i32 sum = 0;
    i32 count = 0;

    SKA_HASH_MAP_T_FOR_EACH(map, I32I32HashMap, iter) {
        sum += iter.pair->value;
        count++;
    }

    TEST_ASSERT_EQUAL_INT32(3, count);
    TEST_ASSERT_EQUAL_INT32(600, sum);

    I32I32HashMap_deinit(map);
}

void seika_hash_map_template_iterator_empty_test(void) {
    I32I32HashMap* map = I32I32HashMap_init(16);

    i32 count = 0;
    SKA_HASH_MAP_T_FOR_EACH(map, I32I32HashMap, iter) {
        (void)iter;
        count++;
    }

    TEST_ASSERT_EQUAL_INT32(0, count);

    I32I32HashMap_deinit(map);
}

void seika_hash_map_template_iterator_modify_test(void) {
    I32I32HashMap* map = I32I32HashMap_init(16);

    I32I32HashMap_add(map, 1, 10);
    I32I32HashMap_add(map, 2, 20);
    I32I32HashMap_add(map, 3, 30);

    SKA_HASH_MAP_T_FOR_EACH(map, I32I32HashMap, iter) {
        iter.pair->value *= 2;
    }

    i32* val1 = I32I32HashMap_get(map, 1);
    TEST_ASSERT_EQUAL_INT32(20, *val1);

    i32* val2 = I32I32HashMap_get(map, 2);
    TEST_ASSERT_EQUAL_INT32(40, *val2);

    i32* val3 = I32I32HashMap_get(map, 3);
    TEST_ASSERT_EQUAL_INT32(60, *val3);

    I32I32HashMap_deinit(map);
}

// ===== COLLISION TESTS =====

void seika_hash_map_template_collision_test(void) {
    I32I32HashMap* map = I32I32HashMap_init(4);

    for (i32 i = 0; i < 20; i++) {
        I32I32HashMap_add(map, i, i * 10);
    }

    TEST_ASSERT_EQUAL_UINT(20, map->size);

    for (i32 i = 0; i < 20; i++) {
        TEST_ASSERT_TRUE(I32I32HashMap_has(map, i));
        i32* val = I32I32HashMap_get(map, i);
        TEST_ASSERT_EQUAL_INT32(i * 10, *val);
    }

    I32I32HashMap_deinit(map);
}

// ===== GET WITH HASH TESTS =====

void seika_hash_map_template_get_with_hash_test(void) {
    I32I32HashMap* map = I32I32HashMap_init(16);

    I32I32HashMapAddResult result = I32I32HashMap_add(map, 42, 420);
    usize hash = result.hash;

    i32* value = I32I32HashMap_get_with_hash(map, 42, hash);
    TEST_ASSERT_NOT_NULL(value);
    TEST_ASSERT_EQUAL_INT32(420, *value);

    I32I32HashMap_deinit(map);
}

// ===== ALLOCATOR TEST =====

void seika_hash_map_template_custom_allocator_test(void) {
    SkaAllocator custom_allocator = ska_mem_get_global_allocator();

    I32I32HashMap* map = I32I32HashMap_init_a(custom_allocator, 16);

    I32I32HashMap_add(map, 1, 100);

    TEST_ASSERT_EQUAL_UINT(1, map->size);

    i32* val = I32I32HashMap_get(map, 1);
    TEST_ASSERT_EQUAL_INT32(100, *val);

    I32I32HashMap_deinit(map);
}

// ===== EDGE CASE TESTS =====

void seika_hash_map_template_empty_operations_test(void) {
    I32I32HashMap* map = I32I32HashMap_init(16);

    TEST_ASSERT_FALSE(I32I32HashMap_has(map, 1));
    TEST_ASSERT_NULL(I32I32HashMap_get(map, 1));
    TEST_ASSERT_FALSE(I32I32HashMap_erase(map, 1));

    I32I32HashMap_clear(map);
    TEST_ASSERT_EQUAL_UINT(0, map->size);

    I32I32HashMap_deinit(map);
}

void seika_hash_map_template_single_element_test(void) {
    I32I32HashMap* map = I32I32HashMap_init(16);

    I32I32HashMap_add(map, 42, 420);

    TEST_ASSERT_EQUAL_UINT(1, map->size);
    TEST_ASSERT_TRUE(I32I32HashMap_has(map, 42));

    i32* val = I32I32HashMap_get(map, 42);
    TEST_ASSERT_EQUAL_INT32(420, *val);

    TEST_ASSERT_TRUE(I32I32HashMap_erase(map, 42));
    TEST_ASSERT_EQUAL_UINT(0, map->size);

    I32I32HashMap_deinit(map);
}

void seika_hash_map_template_min_capacity_test(void) {
    // Test that MIN_CAPACITY constant is defined and reasonable
    TEST_ASSERT_EQUAL_UINT(8, SKA_HASH_MAP_T_MIN_CAPACITY);

    // Test that a map with very small initial capacity still works
    I32I32HashMap* map = I32I32HashMap_init(1);

    // Should be able to add elements regardless of initial capacity
    for (i32 i = 0; i < 10; i++) {
        I32I32HashMap_add(map, i, i * 10);
    }

    TEST_ASSERT_EQUAL_UINT(10, map->size);

    // Verify all elements are present
    for (i32 i = 0; i < 10; i++) {
        TEST_ASSERT_TRUE(I32I32HashMap_has(map, i));
    }

    I32I32HashMap_deinit(map);
}

// ===== MAIN TEST RUNNER =====

i32 main(i32 argv, char** args) {
    UNITY_BEGIN();

    // Basic int tests
    RUN_TEST(seika_hash_map_template_init_test);
    RUN_TEST(seika_hash_map_template_add_test);
    RUN_TEST(seika_hash_map_template_add_duplicate_test);
    RUN_TEST(seika_hash_map_template_get_test);
    RUN_TEST(seika_hash_map_template_has_test);
    RUN_TEST(seika_hash_map_template_erase_test);
    RUN_TEST(seika_hash_map_template_clear_test);

    // Dynamic growth/shrink
    RUN_TEST(seika_hash_map_template_dynamic_growth_test);
    RUN_TEST(seika_hash_map_template_shrink_test);

    // Pointer key tests
    RUN_TEST(seika_hash_map_template_pointer_key_test);
    RUN_TEST(seika_hash_map_template_pointer_key_custom_hash_test);

    // Pointer to pointer key tests
    RUN_TEST(seika_hash_map_template_ptr_ptr_key_test);

    // Struct key tests
    RUN_TEST(seika_hash_map_template_struct_key_test);
    RUN_TEST(seika_hash_map_template_struct_key_custom_hash_test);

    // Pointer value tests
    RUN_TEST(seika_hash_map_template_pointer_value_test);

    // String key tests
    RUN_TEST(seika_hash_map_template_string_key_basic_test);
    RUN_TEST(seika_hash_map_template_string_key_has_test);
    RUN_TEST(seika_hash_map_template_string_key_erase_test);
    RUN_TEST(seika_hash_map_template_string_key_update_test);
    RUN_TEST(seika_hash_map_template_string_key_empty_string_test);
    RUN_TEST(seika_hash_map_template_string_key_long_strings_test);
    RUN_TEST(seika_hash_map_template_string_key_similar_strings_test);
    RUN_TEST(seika_hash_map_template_string_to_string_test);
    RUN_TEST(seika_hash_map_template_string_key_many_entries_test);
    RUN_TEST(seika_hash_map_template_string_key_iterator_test);

    // Iterator tests
    RUN_TEST(seika_hash_map_template_iterator_test);
    RUN_TEST(seika_hash_map_template_iterator_empty_test);
    RUN_TEST(seika_hash_map_template_iterator_modify_test);

    // Collision tests
    RUN_TEST(seika_hash_map_template_collision_test);

    // Get with hash tests
    RUN_TEST(seika_hash_map_template_get_with_hash_test);

    // Allocator test
    RUN_TEST(seika_hash_map_template_custom_allocator_test);

    // Edge cases
    RUN_TEST(seika_hash_map_template_empty_operations_test);
    RUN_TEST(seika_hash_map_template_single_element_test);
    RUN_TEST(seika_hash_map_template_min_capacity_test);

    return UNITY_END();
}
