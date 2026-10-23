#include <unity.h>
#include <seika/string.h>
#include <seika/memory/arena_allocator.h>
#include <seika/memory/pool_allocator.h>
#include <seika/data_structures/array_list.h>
#include <seika/data_structures/hash_map.h>
#include <seika/data_structures/spatial_hash_map.h>

void setUp(void) {}
void tearDown(void) {}

void seika_arena_allocator_test(void);
void seika_allocator_interface_test(void);
void seika_pool_allocator_test(void);
void seika_array_list_test(void);
void seika_hash_map_test(void);
void seika_string_test(void);
void seika_string_enhanced_test(void);
void seika_spatial_hash_map_test(void);
void seika_spatial_hash_map_tall_object_overlap_test(void);

int32 main(int32 argv, char** args) {
    UNITY_BEGIN();
    RUN_TEST(seika_arena_allocator_test);
    RUN_TEST(seika_allocator_interface_test);
    RUN_TEST(seika_pool_allocator_test);
    RUN_TEST(seika_array_list_test);
    RUN_TEST(seika_hash_map_test);
    RUN_TEST(seika_string_test);
    RUN_TEST(seika_string_enhanced_test);
    RUN_TEST(seika_spatial_hash_map_test);
    RUN_TEST(seika_spatial_hash_map_tall_object_overlap_test);
    return UNITY_END();
}

void seika_arena_allocator_test(void) {
    // Create arena
    SkaArena* arena = ska_arena_init(1024);
    TEST_ASSERT_NOT_NULL(arena);
    TEST_ASSERT_EQUAL_UINT(1024, arena->capacity);
    TEST_ASSERT_EQUAL_UINT(0, arena->offset);

    // Allocate
    int32* num = (int32*)ska_arena_alloc(arena, sizeof(int32));
    TEST_ASSERT_NOT_NULL(num);
    *num = 42;
    TEST_ASSERT_EQUAL_INT32(42, *num);

    // Allocate array
    int32* array = (int32*)ska_arena_alloc(arena, sizeof(int32) * 10);
    TEST_ASSERT_NOT_NULL(array);
    array[0] = 100;
    array[9] = 999;
    TEST_ASSERT_EQUAL_INT32(100, array[0]);
    TEST_ASSERT_EQUAL_INT32(999, array[9]);

    // Test reset
    ska_arena_reset(arena);
    TEST_ASSERT_EQUAL_UINT(0, ska_arena_get_used(arena));

    // Cleanup
    ska_arena_deinit(arena);
}

void seika_allocator_interface_test(void) {
    // Test standard allocator
    SkaAllocator alloc = ska_mem_standard_allocator();

    int32* num = (int32*)alloc.allocate(&alloc, sizeof(int32));
    TEST_ASSERT_NOT_NULL(num);
    *num = 123;
    TEST_ASSERT_EQUAL_INT32(123, *num);
    alloc.free(&alloc, num);

    // Test global macros
    int32* global = SKA_ALLOC(int32);
    TEST_ASSERT_NOT_NULL(global);
    *global = 777;
    TEST_ASSERT_EQUAL_INT32(777, *global);
    SKA_FREE(global);
}

void seika_pool_allocator_test(void) {
    // Test pool creation
    SkaPool* pool = ska_pool_init(sizeof(int32), 10);
    TEST_ASSERT_NOT_NULL(pool);
    TEST_ASSERT_EQUAL_UINT(10, ska_pool_get_total_count(pool));
    TEST_ASSERT_EQUAL_UINT(10, ska_pool_get_free_count(pool));
    TEST_ASSERT_EQUAL_UINT(0, ska_pool_get_used_count(pool));
    TEST_ASSERT_TRUE(ska_pool_is_empty(pool));
    TEST_ASSERT_FALSE(ska_pool_is_full(pool));

    // Test allocation
    int32* a = (int32*)ska_pool_alloc(pool);
    TEST_ASSERT_NOT_NULL(a);
    *a = 100;
    TEST_ASSERT_EQUAL_UINT(1, ska_pool_get_used_count(pool));
    TEST_ASSERT_EQUAL_UINT(9, ska_pool_get_free_count(pool));

    int32* b = (int32*)ska_pool_alloc(pool);
    TEST_ASSERT_NOT_NULL(b);
    *b = 200;
    TEST_ASSERT_EQUAL_UINT(2, ska_pool_get_used_count(pool));

    // Values should be preserved
    TEST_ASSERT_EQUAL_INT32(100, *a);
    TEST_ASSERT_EQUAL_INT32(200, *b);

    // Test free
    ska_pool_free(pool, a);
    TEST_ASSERT_EQUAL_UINT(1, ska_pool_get_used_count(pool));
    TEST_ASSERT_EQUAL_UINT(9, ska_pool_get_free_count(pool));

    // Can reallocate freed block
    int32* c = (int32*)ska_pool_alloc(pool);
    TEST_ASSERT_NOT_NULL(c);
    *c = 300;
    TEST_ASSERT_EQUAL_INT32(300, *c);

    // Test multiple allocations
    int32* blocks[8];
    for (int32 i = 0; i < 8; i++) {
        blocks[i] = (int32*)ska_pool_alloc(pool);
        TEST_ASSERT_NOT_NULL(blocks[i]);
        *blocks[i] = i * 10;
    }
    TEST_ASSERT_EQUAL_UINT(10, ska_pool_get_used_count(pool));
    TEST_ASSERT_TRUE(ska_pool_is_full(pool));

    // Pool should be full now
    int32* should_fail = (int32*)ska_pool_alloc(pool);
    TEST_ASSERT_NULL(should_fail);

    // Test owns_block
    TEST_ASSERT_TRUE(ska_pool_owns_block(pool, b));
    TEST_ASSERT_TRUE(ska_pool_owns_block(pool, c));

    int32 external_value = 999;
    TEST_ASSERT_FALSE(ska_pool_owns_block(pool, &external_value));

    // Test reset
    ska_pool_reset(pool);
    TEST_ASSERT_EQUAL_UINT(0, ska_pool_get_used_count(pool));
    TEST_ASSERT_EQUAL_UINT(10, ska_pool_get_free_count(pool));
    TEST_ASSERT_TRUE(ska_pool_is_empty(pool));

    // Can allocate again after reset
    int32* d = (int32*)ska_pool_alloc(pool);
    TEST_ASSERT_NOT_NULL(d);
    *d = 777;
    TEST_ASSERT_EQUAL_INT32(777, *d);

    // Cleanup
    ska_pool_deinit(pool);
}

void seika_array_list_test(void) {
    // Test init and deinit
    SkaArrayList list = ska_array_list_init(sizeof(int32), 10);


    TEST_ASSERT_NOT_NULL(list.data);
    TEST_ASSERT_EQUAL_UINT(sizeof(int32), list.value_size);
    TEST_ASSERT_EQUAL_UINT(0, list.size);
    TEST_ASSERT_EQUAL_UINT(10, list.capacity);
    TEST_ASSERT_EQUAL_UINT(10, list.initial_capacity);

    ska_array_list_deinit(&list);
    TEST_ASSERT_NULL(list.data);
    TEST_ASSERT_EQUAL_UINT(0, list.size);

    // Test for-each with integers (value type)
    SkaArrayList int_list = ska_array_list_init(sizeof(int32), 4);
    ska_array_list_append(&int_list, &(int32){10});
    ska_array_list_append(&int_list, &(int32){20});
    ska_array_list_append(&int_list, &(int32){30});

    int32 sum = 0;
    SKA_ARRAY_LIST_FOR_EACH(&int_list, int32*, iter) {
        sum += *iter;
    }
    TEST_ASSERT_EQUAL_INT32(60, sum);
    ska_array_list_deinit(&int_list);

    // Test for-each with struct (value type)
    typedef struct TestObject {
        int32 id;
        int32 value;
    } TestObject;

    SkaArrayList obj_list = ska_array_list_init(sizeof(TestObject), 4);
    ska_array_list_append(&obj_list, &(TestObject){.id = 1, .value = 100});
    ska_array_list_append(&obj_list, &(TestObject){.id = 2, .value = 200});
    ska_array_list_append(&obj_list, &(TestObject){.id = 3, .value = 300});

    int32 id_sum = 0;
    int32 value_sum = 0;
    SKA_ARRAY_LIST_FOR_EACH(&obj_list, TestObject*, iter) {
        id_sum += iter->id;
        value_sum += iter->value;
    }
    TEST_ASSERT_EQUAL_INT32(6, id_sum);
    TEST_ASSERT_EQUAL_INT32(600, value_sum);
    ska_array_list_deinit(&obj_list);

    // Test for-each with pointers (pointer to pointer)
    SkaArrayList ptr_list = ska_array_list_init(sizeof(int32*), 4);

    int32* ptr1 = SKA_ALLOC(int32);
    *ptr1 = 111;
    int32* ptr2 = SKA_ALLOC(int32);
    *ptr2 = 222;
    int32* ptr3 = SKA_ALLOC(int32);
    *ptr3 = 333;

    ska_array_list_append(&ptr_list, &ptr1);
    ska_array_list_append(&ptr_list, &ptr2);
    ska_array_list_append(&ptr_list, &ptr3);

    int32 ptr_sum = 0;
    SKA_ARRAY_LIST_FOR_EACH(&ptr_list, int32**, iter) {
        ptr_sum += **iter;
    }
    TEST_ASSERT_EQUAL_INT32(666, ptr_sum);

    // Cleanup pointers
    SKA_FREE(ptr1);
    SKA_FREE(ptr2);
    SKA_FREE(ptr3);
    ska_array_list_deinit(&ptr_list);
}

void seika_hash_map_test(void) {
    // Test init and deinit
    SkaHashMap* map = ska_hash_map_init(sizeof(int32), sizeof(int32), 8);
    TEST_ASSERT_NOT_NULL(map);
    TEST_ASSERT_EQUAL_UINT(sizeof(int32), map->key_size);
    TEST_ASSERT_EQUAL_UINT(sizeof(int32), map->value_size);
    TEST_ASSERT_EQUAL_UINT(0, map->size);
    TEST_ASSERT_EQUAL_UINT(8, map->capacity);
    TEST_ASSERT_EQUAL_UINT(8, map->initial_capacity);
    TEST_ASSERT_NOT_NULL(map->hash_func);
    TEST_ASSERT_NOT_NULL(map->compare_func);

    ska_hash_map_deinit(map);

    // Test add and get
    map = ska_hash_map_init(sizeof(int32), sizeof(int32), 8);

    int32 key1 = 10;
    int32 value1 = 100;
    TEST_ASSERT_TRUE(ska_hash_map_add(map, &key1, &value1).was_added);
    TEST_ASSERT_EQUAL_UINT(1, map->size);

    int32* retrieved = (int32*)ska_hash_map_get(map, &key1);
    TEST_ASSERT_NOT_NULL(retrieved);
    TEST_ASSERT_EQUAL_INT32(100, *retrieved);

    // Test update existing key
    int32 value2 = 200;
    TEST_ASSERT_FALSE(ska_hash_map_add(map, &key1, &value2).was_added); // Update failed
    TEST_ASSERT_EQUAL_UINT(1, map->size); // Size should not increase

    retrieved = (int32*)ska_hash_map_get(map, &key1);
    TEST_ASSERT_NOT_NULL(retrieved);
    TEST_ASSERT_EQUAL_INT32(100, *retrieved);

    // Test multiple keys
    int32 key2 = 20;
    int32 value3 = 300;
    TEST_ASSERT_TRUE(ska_hash_map_add(map, &key2, &value3).was_added);
    TEST_ASSERT_EQUAL_UINT(2, map->size);

    int32 key3 = 30;
    int32 value4 = 400;
    TEST_ASSERT_TRUE(ska_hash_map_add(map, &key3, &value4).was_added);
    TEST_ASSERT_EQUAL_UINT(3, map->size);

    // Test has
    TEST_ASSERT_TRUE(ska_hash_map_has(map, &key1));
    TEST_ASSERT_TRUE(ska_hash_map_has(map, &key2));
    TEST_ASSERT_TRUE(ska_hash_map_has(map, &key3));

    int32 non_existent_key = 999;
    TEST_ASSERT_FALSE(ska_hash_map_has(map, &non_existent_key));

    // Test get non-existent key
    int32* non_existent = (int32*)ska_hash_map_get(map, &non_existent_key);
    TEST_ASSERT_NULL(non_existent);

    // Test erase
    TEST_ASSERT_TRUE(ska_hash_map_erase(map, &key2));
    TEST_ASSERT_EQUAL_UINT(2, map->size);
    TEST_ASSERT_FALSE(ska_hash_map_has(map, &key2));
    TEST_ASSERT_NULL(ska_hash_map_get(map, &key2));

    // Test erase non-existent key
    TEST_ASSERT_FALSE(ska_hash_map_erase(map, &non_existent_key));
    TEST_ASSERT_EQUAL_UINT(2, map->size);

    ska_hash_map_deinit(map);

    // Test with string keys and values
    map = ska_hash_map_init(sizeof(char*), sizeof(int32), 8);

    const char* str_key1 = "apple";
    int32 str_value1 = 1;
    ska_hash_map_add(map, (void*)&str_key1, &str_value1);

    const char* str_key2 = "banana";
    int32 str_value2 = 2;
    ska_hash_map_add(map, (void*)&str_key2, &str_value2);

    int32* str_retrieved = (int32*)ska_hash_map_get(map, (void*)&str_key1);
    TEST_ASSERT_NOT_NULL(str_retrieved);
    TEST_ASSERT_EQUAL_INT32(1, *str_retrieved);

    ska_hash_map_deinit(map);

    // Test growth (resize on insertion)
    map = ska_hash_map_init(sizeof(int32), sizeof(int32), 4);
    for (int32 i = 0; i < 10; i++) {
        int32 value = i * 10;
        ska_hash_map_add(map, &i, &value);
    }
    TEST_ASSERT_EQUAL_UINT(10, map->size);
    TEST_ASSERT_TRUE(map->capacity >= 10);

    // Verify all values are still accessible after resize
    for (int32 i = 0; i < 10; i++) {
        int32* val = (int32*)ska_hash_map_get(map, &i);
        TEST_ASSERT_NOT_NULL(val);
        TEST_ASSERT_EQUAL_INT32(i * 10, *val);
    }

    ska_hash_map_deinit(map);

    // Test clear
    map = ska_hash_map_init(sizeof(int32), sizeof(int32), 8);
    for (int32 i = 0; i < 5; i++) {
        int32 value = i;
        ska_hash_map_add(map, &i, &value);
    }
    TEST_ASSERT_EQUAL_UINT(5, map->size);

    ska_hash_map_clear(map);
    TEST_ASSERT_EQUAL_UINT(0, map->size);

    int32 test_key = 0;
    TEST_ASSERT_FALSE(ska_hash_map_has(map, &test_key));
    TEST_ASSERT_NULL(ska_hash_map_get(map, &test_key));

    ska_hash_map_deinit(map);

    // Test iterator
    map = ska_hash_map_init(sizeof(int32), sizeof(int32), 8);

    for (int32 i = 0; i < 5; i++) {
        int32 value = i * 100;
        ska_hash_map_add(map, &i, &value);
    }

    int32 iter_count = 0;
    int32 iter_sum = 0;
    SKA_HASH_MAP_FOR_EACH(map, iter) {
        int32* key = (int32*)iter.pair->key;
        int32* value = (int32*)iter.pair->value;
        TEST_ASSERT_NOT_NULL(key);
        TEST_ASSERT_NOT_NULL(value);
        TEST_ASSERT_EQUAL_INT32(*key * 100, *value);
        iter_count++;
        iter_sum += *value;
    }
    TEST_ASSERT_EQUAL_INT32(5, iter_count);
    TEST_ASSERT_EQUAL_INT32(1000, iter_sum); // 0 + 100 + 200 + 300 + 400

    // Test iterator on empty map
    ska_hash_map_clear(map);
    iter_count = 0;
    SKA_HASH_MAP_FOR_EACH(map, iter) {
        iter_count++;
    }
    TEST_ASSERT_EQUAL_INT32(0, iter_count);

    ska_hash_map_deinit(map);

    // Test collision handling (multiple keys hashing to same bucket)
    map = ska_hash_map_init(sizeof(int32), sizeof(int32), 2);

    // Add multiple items that will cause collisions
    for (int32 i = 0; i < 6; i++) {
        int32 value = i * 50;
        ska_hash_map_add(map, &i, &value);
    }
    TEST_ASSERT_EQUAL_UINT(6, map->size);

    // Verify all can be retrieved
    for (int32 i = 0; i < 6; i++) {
        int32* val = (int32*)ska_hash_map_get(map, &i);
        TEST_ASSERT_NOT_NULL(val);
        TEST_ASSERT_EQUAL_INT32(i * 50, *val);
    }

    ska_hash_map_deinit(map);

    // Test struct values
    typedef struct TestStruct {
        int32 id;
        float score;
    } TestStruct;

    map = ska_hash_map_init(sizeof(int32), sizeof(TestStruct), 8);

    int32 struct_key = 42;
    TestStruct struct_value = {.id = 100, .score = 95.5f};
    ska_hash_map_add(map, &struct_key, &struct_value);

    TestStruct* retrieved_struct = (TestStruct*)ska_hash_map_get(map, &struct_key);
    TEST_ASSERT_NOT_NULL(retrieved_struct);
    TEST_ASSERT_EQUAL_INT32(100, retrieved_struct->id);
    TEST_ASSERT_EQUAL_FLOAT(95.5f, retrieved_struct->score);

    ska_hash_map_deinit(map);
}

void seika_string_test(void) {
    // Test SSO initialization
    SkaString str1;
    ska_string_init(&str1, "Hello", 5);
    TEST_ASSERT_EQUAL_UINT(5, str1.len);
    TEST_ASSERT_FALSE(str1.is_heap);
    TEST_ASSERT_EQUAL_STRING("Hello", ska_string_as_slice(&str1));
    ska_string_deinit(&str1);

    // Test heap allocation (exceeds SSO capacity of 22)
    const char* long_text = "This is a very long string";
    SkaString str2;
    ska_string_init(&str2, long_text, ska_strlen(long_text));
    TEST_ASSERT_EQUAL_UINT(26, str2.len);
    TEST_ASSERT_TRUE(str2.is_heap);
    ska_string_deinit(&str2);

    // Test append
    SkaString str3;
    ska_string_init(&str3, "Hello", 5);
    ska_string_append(&str3, " World", 6);
    TEST_ASSERT_EQUAL_UINT(11, str3.len);
    TEST_ASSERT_EQUAL_STRING("Hello World", ska_string_as_slice(&str3));
    ska_string_deinit(&str3);

    // Test set
    SkaString str4;
    ska_string_init(&str4, "First", 5);
    ska_string_set(&str4, "Second", 6);
    TEST_ASSERT_EQUAL_STRING("Second", ska_string_as_slice(&str4));
    ska_string_deinit(&str4);

    // Test equality
    SkaString str5, str6;
    ska_string_init(&str5, "Equal", 5);
    ska_string_init(&str6, "Equal", 5);
    TEST_ASSERT_TRUE(ska_string_eql(&str5, &str6));
    TEST_ASSERT_TRUE(ska_string_eql_cstr(&str5, "Equal", 5));
    ska_string_deinit(&str5);
    ska_string_deinit(&str6);

    // Test clone
    SkaString str7, str8;
    ska_string_init(&str7, "Original", 8);
    ska_string_copy(&str7, &str8);
    TEST_ASSERT_TRUE(ska_string_eql(&str7, &str8));
    ska_string_deinit(&str7);
    ska_string_deinit(&str8);

    // Test clear
    SkaString str9;
    ska_string_init(&str9, "Clear me", 8);
    ska_string_clear(&str9);
    TEST_ASSERT_EQUAL_UINT(0, str9.len);
    ska_string_deinit(&str9);
}

void seika_string_enhanced_test(void) {
    // Test init from C string
    SkaString str1;
    ska_string_init_cstr(&str1, "Hello World");
    TEST_ASSERT_EQUAL_UINT(11, str1.len);
    TEST_ASSERT_EQUAL_STRING("Hello World", ska_string_as_slice(&str1));
    ska_string_deinit(&str1);

    // Test init from substring
    const char* source = "Hello World";
    SkaString str2;
    ska_string_init_substring(&str2, source + 6, 5); // "World"
    TEST_ASSERT_EQUAL_UINT(5, str2.len);
    TEST_ASSERT_EQUAL_STRING("World", ska_string_as_slice(&str2));
    ska_string_deinit(&str2);

    // Test trim whitespace (both ends)
    SkaString str3;
    ska_string_init_cstr(&str3, "  Hello World  ");
    usize new_len = ska_string_trim_whitespace(&str3);
    TEST_ASSERT_EQUAL_UINT(11, new_len);
    TEST_ASSERT_EQUAL_UINT(11, str3.len);
    TEST_ASSERT_EQUAL_STRING("Hello World", ska_string_as_slice(&str3));
    ska_string_deinit(&str3);

    // Test trim whitespace (already trimmed)
    SkaString str4;
    ska_string_init_cstr(&str4, "NoSpaces");
    new_len = ska_string_trim_whitespace(&str4);
    TEST_ASSERT_EQUAL_UINT(8, new_len);
    TEST_ASSERT_EQUAL_STRING("NoSpaces", ska_string_as_slice(&str4));
    ska_string_deinit(&str4);

    // Test trim whitespace (all whitespace)
    SkaString str5;
    ska_string_init_cstr(&str5, "   \t\n  ");
    new_len = ska_string_trim_whitespace(&str5);
    TEST_ASSERT_EQUAL_UINT(0, new_len);
    TEST_ASSERT_EQUAL_UINT(0, str5.len);
    ska_string_deinit(&str5);

    // Test trim start
    SkaString str6;
    ska_string_init_cstr(&str6, "  Leading");
    new_len = ska_string_trim_start(&str6);
    TEST_ASSERT_EQUAL_UINT(7, new_len);
    TEST_ASSERT_EQUAL_STRING("Leading", ska_string_as_slice(&str6));
    ska_string_deinit(&str6);

    // Test trim end
    SkaString str7;
    ska_string_init_cstr(&str7, "Trailing  ");
    new_len = ska_string_trim_end(&str7);
    TEST_ASSERT_EQUAL_UINT(8, new_len);
    TEST_ASSERT_EQUAL_STRING("Trailing", ska_string_as_slice(&str7));
    ska_string_deinit(&str7);

    // Test substring view
    SkaString str8;
    ska_string_init_cstr(&str8, "Hello World");
    usize out_len;
    const char* view = ska_string_substring_view(&str8, 6, 5, &out_len);
    TEST_ASSERT_NOT_NULL(view);
    TEST_ASSERT_EQUAL_UINT(5, out_len);
    TEST_ASSERT_EQUAL_MEMORY("World", view, 5);
    ska_string_deinit(&str8);

    // Test substring view (out of bounds)
    SkaString str9;
    ska_string_init_cstr(&str9, "Short");
    view = ska_string_substring_view(&str9, 10, 5, &out_len);
    TEST_ASSERT_NULL(view);
    TEST_ASSERT_EQUAL_UINT(0, out_len);
    ska_string_deinit(&str9);

    // Test substring view (partial out of bounds)
    SkaString str10;
    ska_string_init_cstr(&str10, "Hello");
    view = ska_string_substring_view(&str10, 3, 10, &out_len);
    TEST_ASSERT_NOT_NULL(view);
    TEST_ASSERT_EQUAL_UINT(2, out_len); // Only "lo" available
    TEST_ASSERT_EQUAL_MEMORY("lo", view, 2);
    ska_string_deinit(&str10);

    // Test substring copy
    SkaString str11, substr1;
    ska_string_init_cstr(&str11, "Hello World");
    ska_string_substring_copy(&str11, 0, 5, &substr1);
    TEST_ASSERT_EQUAL_UINT(5, substr1.len);
    TEST_ASSERT_EQUAL_STRING("Hello", ska_string_as_slice(&substr1));
    ska_string_deinit(&str11);
    ska_string_deinit(&substr1);

    // Test find char
    SkaString str12;
    ska_string_init_cstr(&str12, "Hello World");
    i64 pos = ska_string_find_char(&str12, 'o');
    TEST_ASSERT_EQUAL_INT64(4, pos); // First 'o' in "Hello"

    pos = ska_string_find_char(&str12, 'W');
    TEST_ASSERT_EQUAL_INT64(6, pos);

    pos = ska_string_find_char(&str12, 'z');
    TEST_ASSERT_EQUAL_INT64(-1, pos); // Not found
    ska_string_deinit(&str12);

    // Test find char last
    SkaString str13;
    ska_string_init_cstr(&str13, "Hello World");
    pos = ska_string_find_char_last(&str13, 'o');
    TEST_ASSERT_EQUAL_INT64(7, pos); // Last 'o' in "World"

    pos = ska_string_find_char_last(&str13, 'H');
    TEST_ASSERT_EQUAL_INT64(0, pos);

    pos = ska_string_find_char_last(&str13, 'z');
    TEST_ASSERT_EQUAL_INT64(-1, pos); // Not found
    ska_string_deinit(&str13);

    // Test starts with
    SkaString str14;
    ska_string_init_cstr(&str14, "Hello World");
    TEST_ASSERT_TRUE(ska_string_starts_with(&str14, "Hello", 5));
    TEST_ASSERT_TRUE(ska_string_starts_with(&str14, "H", 1));
    TEST_ASSERT_FALSE(ska_string_starts_with(&str14, "World", 5));
    TEST_ASSERT_FALSE(ska_string_starts_with(&str14, "Hello World!", 12)); // Too long
    ska_string_deinit(&str14);

    // Test ends with
    SkaString str15;
    ska_string_init_cstr(&str15, "Hello World");
    TEST_ASSERT_TRUE(ska_string_ends_with(&str15, "World", 5));
    TEST_ASSERT_TRUE(ska_string_ends_with(&str15, "d", 1));
    TEST_ASSERT_FALSE(ska_string_ends_with(&str15, "Hello", 5));
    TEST_ASSERT_FALSE(ska_string_ends_with(&str15, "Hello World!", 12)); // Too long
    ska_string_deinit(&str15);

    // Test char is whitespace
    TEST_ASSERT_TRUE(ska_char_is_whitespace(' '));
    TEST_ASSERT_TRUE(ska_char_is_whitespace('\t'));
    TEST_ASSERT_TRUE(ska_char_is_whitespace('\n'));
    TEST_ASSERT_TRUE(ska_char_is_whitespace('\r'));
    TEST_ASSERT_FALSE(ska_char_is_whitespace('a'));
    TEST_ASSERT_FALSE(ska_char_is_whitespace('0'));
    TEST_ASSERT_FALSE(ska_char_is_whitespace('\0'));

    // Test ensure null terminated (SSO)
    SkaString str16;
    ska_string_init_cstr(&str16, "Short");
    TEST_ASSERT_FALSE(str16.is_heap);
    TEST_ASSERT_TRUE(ska_string_ensure_null_terminated(&str16));
    // Verify it's actually null terminated
    const char* slice = ska_string_as_slice(&str16);
    TEST_ASSERT_EQUAL_CHAR('\0', slice[str16.len]);
    ska_string_deinit(&str16);

    // Heap strings are null-terminated too (they weren't, so as_slice read past the buffer)
    SkaString str17;
    ska_string_init_cstr(&str17, "This is a very long string that exceeds SSO capacity");
    TEST_ASSERT_TRUE(str17.is_heap);
    TEST_ASSERT_EQUAL_CHAR('\0', ska_string_as_slice(&str17)[str17.len]);
    TEST_ASSERT_TRUE(ska_string_ensure_null_terminated(&str17));
    ska_string_deinit(&str17);

    // Appending across the SSO boundary, and exactly filling SSO, keep a terminator
    SkaString str17b;
    ska_string_init_empty(&str17b);
    ska_string_append(&str17b, "0123456789", 10);
    ska_string_append(&str17b, "0123456789AB", 12); // exactly SKA_STRING_SSO_CAPACITY
    TEST_ASSERT_FALSE(str17b.is_heap);
    TEST_ASSERT_EQUAL_STRING("01234567890123456789AB", ska_string_as_slice(&str17b));
    ska_string_append(&str17b, "-tail", 5);
    TEST_ASSERT_TRUE(str17b.is_heap);
    TEST_ASSERT_EQUAL_STRING("01234567890123456789AB-tail", ska_string_as_slice(&str17b));
    ska_string_trim_end(&str17b);
    ska_string_append(&str17b, " end ", 5);
    ska_string_trim_whitespace(&str17b);
    TEST_ASSERT_EQUAL_STRING("01234567890123456789AB-tail end", ska_string_as_slice(&str17b));
    ska_string_deinit(&str17b);

    // Test get writable (SSO)
    SkaString str18;
    ska_string_init_cstr(&str18, "Mutable");
    char* writable = ska_string_get_writable(&str18);
    TEST_ASSERT_NOT_NULL(writable);
    // Modify in place
    writable[0] = 'm'; // "mutable"
    TEST_ASSERT_EQUAL_CHAR('m', ska_string_as_slice(&str18)[0]);
    ska_string_deinit(&str18);

    // Test complex trimming scenario (simulates f-string parsing)
    SkaString str19;
    ska_string_init_cstr(&str19, "\n\t  name  \t\n");
    ska_string_trim_whitespace(&str19);
    TEST_ASSERT_EQUAL_UINT(4, str19.len);
    TEST_ASSERT_EQUAL_STRING("name", ska_string_as_slice(&str19));
    ska_string_deinit(&str19);

    // Test empty string operations
    SkaString str20;
    ska_string_init_empty(&str20);
    TEST_ASSERT_EQUAL_UINT(0, str20.len);

    // Trim empty string
    ska_string_trim_whitespace(&str20);
    TEST_ASSERT_EQUAL_UINT(0, str20.len);

    // Find in empty string
    pos = ska_string_find_char(&str20, 'a');
    TEST_ASSERT_EQUAL_INT64(-1, pos);

    // Substring view of empty string
    view = ska_string_substring_view(&str20, 0, 5, &out_len);
    TEST_ASSERT_NULL(view);
    TEST_ASSERT_EQUAL_UINT(0, out_len);

    ska_string_deinit(&str20);

    // Test SSO boundary conditions
    SkaString str21;
    ska_string_init_cstr(&str21, "  x  "); // Small, fits in SSO
    TEST_ASSERT_FALSE(str21.is_heap);
    ska_string_trim_whitespace(&str21);
    TEST_ASSERT_EQUAL_UINT(1, str21.len);
    TEST_ASSERT_FALSE(str21.is_heap); // Should still be SSO
    TEST_ASSERT_EQUAL_STRING("x", ska_string_as_slice(&str21));
    ska_string_deinit(&str21);

    // Test multiple operations in sequence
    SkaString str22;
    ska_string_init_cstr(&str22, "  Hello  ");
    ska_string_trim_whitespace(&str22);
    TEST_ASSERT_EQUAL_STRING("Hello", ska_string_as_slice(&str22));

    ska_string_append(&str22, " World", 6);
    TEST_ASSERT_EQUAL_STRING("Hello World", ska_string_as_slice(&str22));

    pos = ska_string_find_char(&str22, ' ');
    TEST_ASSERT_EQUAL_INT64(5, pos);

    TEST_ASSERT_TRUE(ska_string_starts_with(&str22, "Hello", 5));
    TEST_ASSERT_TRUE(ska_string_ends_with(&str22, "World", 5));

    ska_string_deinit(&str22);

    // Test trimming edge cases
    SkaString str23;
    ska_string_init_cstr(&str23, "     NoTrailing");
    ska_string_trim_whitespace(&str23);
    TEST_ASSERT_EQUAL_STRING("NoTrailing", ska_string_as_slice(&str23));
    ska_string_deinit(&str23);

    SkaString str24;
    ska_string_init_cstr(&str24, "NoLeading     ");
    ska_string_trim_whitespace(&str24);
    TEST_ASSERT_EQUAL_STRING("NoLeading", ska_string_as_slice(&str24));
    ska_string_deinit(&str24);

    // Test substring operations
    SkaString str25, substr2;
    ska_string_init_cstr(&str25, "12345");
    ska_string_substring_copy(&str25, 0, 5, &substr2);
    TEST_ASSERT_EQUAL_UINT(5, substr2.len);
    TEST_ASSERT_EQUAL_STRING("12345", ska_string_as_slice(&substr2));
    ska_string_deinit(&str25);
    ska_string_deinit(&substr2);

    SkaString str26, substr3;
    ska_string_init_cstr(&str26, "Hello World");
    ska_string_substring_copy(&str26, 6, 3, &substr3);
    TEST_ASSERT_EQUAL_UINT(3, substr3.len);
    TEST_ASSERT_EQUAL_STRING("Wor", ska_string_as_slice(&substr3));
    ska_string_deinit(&str26);
    ska_string_deinit(&substr3);
}

void seika_spatial_hash_map_test(void) {
    SkaSpatialHashMap* map = ska_spatial_hash_map_create(64);
    TEST_ASSERT_NOT_NULL(map);
    TEST_ASSERT_EQUAL_INT(64, map->cell_size);

    SkaRect2 rect_a = { .x = 0.0f, .y = 0.0f, .w = 10.0f, .h = 10.0f };
    ska_spatial_hash_map_insert_or_update(map, 1, &rect_a);
    SkaRect2 rect_b = { .x = 5.0f, .y = 5.0f, .w = 10.0f, .h = 10.0f };
    ska_spatial_hash_map_insert_or_update(map, 2, &rect_b);

    SkaSpatialHashMapCollisionResult result = ska_spatial_hash_map_compute_collision(map, 1);
    TEST_ASSERT_EQUAL_UINT(1, result.collision_count);
    TEST_ASSERT_EQUAL_UINT(2, result.collisions[0]);

    ska_spatial_hash_map_remove(map, 2);
    result = ska_spatial_hash_map_compute_collision(map, 1);
    TEST_ASSERT_EQUAL_UINT(0, result.collision_count);

    ska_spatial_hash_map_destroy(map);
}

// Regression test for a real bug found via koa's pong example: a Collider2D-conforming paddle
// (16x100) added as a child of its ColorRect never bounced the ball, even on a dead-center hit.
// change_cell_size_if_needed's own doc comment states the invariant it must maintain -- cell_size
// stays at least 2x the largest inserted object -- but the actual comparison checked the opposite
// direction (grew only once an object was *more than double* the cell size, instead of growing as
// soon as the cell size dropped *below* double the object), so a 100-unit-tall object never grew a
// 64-unit cell_size. spatial_hash_map_update only links an object into the grid cells touching its
// 4 corners, not every cell its rect spans, so once an object is taller than the cell size, there's
// a real gap through the middle of its bounding rect that's linked into no cell at all -- exactly
// where this test's small object sits, fully contained within the tall one but only reachable via
// broad-phase from a cell neither one is actually registered in under the old, wrong comparison.
void seika_spatial_hash_map_tall_object_overlap_test(void) {
    SkaSpatialHashMap* map = ska_spatial_hash_map_create(64);

    // Paddle-shaped: taller than the initial 64-unit cell_size, same shape/position pong.koa uses.
    SkaRect2 paddle_rect = { .x = 40.0f, .y = 250.0f, .w = 16.0f, .h = 100.0f };
    ska_spatial_hash_map_insert_or_update(map, 1, &paddle_rect);
    // Sits in the vertical middle of the paddle's rect (250-350), away from either corner's cell.
    SkaRect2 ball_rect = { .x = 40.0f, .y = 292.0f, .w = 16.0f, .h = 16.0f };
    ska_spatial_hash_map_insert_or_update(map, 2, &ball_rect);

    TEST_ASSERT_TRUE_MESSAGE(map->cell_size >= 200,
        "cell_size must grow to at least 2x the 100-unit-tall paddle (this function's own doc comment's invariant)");

    SkaSpatialHashMapCollisionResult result = ska_spatial_hash_map_compute_collision(map, 2);
    TEST_ASSERT_EQUAL_UINT_MESSAGE(1, result.collision_count,
        "a small object fully contained in a tall object's rect must still be found by broad-phase");
    TEST_ASSERT_EQUAL_UINT(1, result.collisions[0]);

    ska_spatial_hash_map_destroy(map);
}
