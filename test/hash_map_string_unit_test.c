#include <unity.h>
#include <seika/data_structures/hash_map_string.h>
#include <seika/memory.h>
#include <string.h>

void setUp(void) {}
void tearDown(void) {}

// ===== Basic add/get/has/erase roundtrip =====

void seika_hash_map_string_basic_roundtrip_test(void) {
    SkaStringHashMap* map = ska_string_hash_map_create_default_capacity();

    i32 value = 42;
    SkaStringHashMapAddResult result = ska_string_hash_map_add(map, "answer", &value, sizeof(i32));
    TEST_ASSERT_TRUE(result.was_added);
    TEST_ASSERT_EQUAL_INT32(42, *(i32*)result.value);

    TEST_ASSERT_TRUE(ska_string_hash_map_has(map, "answer"));
    TEST_ASSERT_FALSE(ska_string_hash_map_has(map, "missing"));

    i32* got = (i32*)ska_string_hash_map_get(map, "answer");
    TEST_ASSERT_NOT_NULL(got);
    TEST_ASSERT_EQUAL_INT32(42, *got);

    TEST_ASSERT_NULL(ska_string_hash_map_get(map, "missing"));

    TEST_ASSERT_TRUE(ska_string_hash_map_erase(map, "answer"));
    TEST_ASSERT_FALSE(ska_string_hash_map_has(map, "answer"));
    TEST_ASSERT_FALSE(ska_string_hash_map_erase(map, "answer"));

    ska_string_hash_map_deinit(map);
}

void seika_hash_map_string_typed_sugar_test(void) {
    SkaStringHashMap* map = ska_string_hash_map_create_default_capacity();

    ska_string_hash_map_add_int(map, "count", 7);
    TEST_ASSERT_EQUAL_INT32(7, ska_string_hash_map_get_int(map, "count"));

    ska_string_hash_map_add_string(map, "greeting", "hello");
    TEST_ASSERT_EQUAL_STRING("hello", ska_string_hash_map_get_string(map, "greeting"));
    // Used to store only sizeof(char*) bytes of the characters, unterminated
    const char* long_value = "file:///c:/some/longer/path/that/exceeds/the/inline/value/capacity/of/the/map/main.koa";
    ska_string_hash_map_add_string(map, "uri", long_value);
    TEST_ASSERT_EQUAL_STRING(long_value, ska_string_hash_map_get_string(map, "uri"));

    ska_string_hash_map_deinit(map);
}

// Duplicate-key add must NOT update the existing value (pre-existing behavior --
// should_update_value has always been hardcoded false) -- redesign must match exactly.
void seika_hash_map_string_duplicate_add_does_not_update_test(void) {
    SkaStringHashMap* map = ska_string_hash_map_create_default_capacity();

    i32 first = 1;
    i32 second = 2;
    SkaStringHashMapAddResult r1 = ska_string_hash_map_add(map, "key", &first, sizeof(i32));
    TEST_ASSERT_TRUE(r1.was_added);

    SkaStringHashMapAddResult r2 = ska_string_hash_map_add(map, "key", &second, sizeof(i32));
    TEST_ASSERT_FALSE(r2.was_added);
    TEST_ASSERT_EQUAL_INT32(1, *(i32*)ska_string_hash_map_get(map, "key"));
    TEST_ASSERT_EQUAL_INT32(1u, map->size);

    ska_string_hash_map_deinit(map);
}

// set() must overwrite an existing key and add a missing one.
void seika_hash_map_string_set_test(void) {
    SkaStringHashMap* map = ska_string_hash_map_create_default_capacity();

    i32 initial = 10;
    TEST_ASSERT_TRUE(ska_string_hash_map_set(map, "x", &initial, sizeof(i32)));
    TEST_ASSERT_EQUAL_INT32(10, *(i32*)ska_string_hash_map_get(map, "x"));

    i32 updated = 20;
    TEST_ASSERT_TRUE(ska_string_hash_map_set(map, "x", &updated, sizeof(i32)));
    TEST_ASSERT_EQUAL_INT32(20, *(i32*)ska_string_hash_map_get(map, "x"));
    TEST_ASSERT_EQUAL_INT32(1u, map->size);

    ska_string_hash_map_deinit(map);
}

// set() transitioning a value across the inline/heap boundary -- highest-risk area of the
// open-addressing redesign. Start small (inline), grow past the inline cap (heap fallback),
// then shrink back below the cap (should stay correct even if still heap-backed).
void seika_hash_map_string_set_across_inline_boundary_test(void) {
    SkaStringHashMap* map = ska_string_hash_map_create_default_capacity();

    char small_value[8] = "small";
    TEST_ASSERT_TRUE(ska_string_hash_map_set(map, "v", small_value, sizeof(small_value)));
    TEST_ASSERT_EQUAL_STRING("small", (char*)ska_string_hash_map_get(map, "v"));

    char large_value[SKA_STRING_HASH_MAP_INLINE_VALUE_CAP + 32];
    memset(large_value, 'A', sizeof(large_value) - 1);
    large_value[sizeof(large_value) - 1] = '\0';
    TEST_ASSERT_TRUE(ska_string_hash_map_set(map, "v", large_value, sizeof(large_value)));
    TEST_ASSERT_EQUAL_STRING(large_value, (char*)ska_string_hash_map_get(map, "v"));

    char small_again[4] = "hi";
    TEST_ASSERT_TRUE(ska_string_hash_map_set(map, "v", small_again, sizeof(small_again)));
    TEST_ASSERT_EQUAL_STRING("hi", (char*)ska_string_hash_map_get(map, "v"));

    ska_string_hash_map_deinit(map);
}

// ===== Iterator =====

void seika_hash_map_string_iterator_full_traversal_test(void) {
    SkaStringHashMap* map = ska_string_hash_map_create_default_capacity();

    const char* keys[] = { "a", "b", "c", "d", "e" };
    for (usize i = 0; i < 5; i++) {
        i32 v = (i32)i;
        ska_string_hash_map_add(map, keys[i], &v, sizeof(i32));
    }

    usize seen = 0;
    SKA_STRING_HASH_MAP_FOR_EACH(map, iter) {
        TEST_ASSERT_NOT_NULL(iter.key);
        TEST_ASSERT_NOT_NULL(iter.value);
        seen++;
    }
    TEST_ASSERT_EQUAL_INT32(5, (i32)seen);

    ska_string_hash_map_deinit(map);
}

void seika_hash_map_string_iterator_skips_tombstones_test(void) {
    SkaStringHashMap* map = ska_string_hash_map_create_default_capacity();

    const char* keys[] = { "a", "b", "c", "d" };
    for (usize i = 0; i < 4; i++) {
        i32 v = (i32)i;
        ska_string_hash_map_add(map, keys[i], &v, sizeof(i32));
    }
    ska_string_hash_map_erase(map, "b");
    ska_string_hash_map_erase(map, "d");

    usize seen = 0;
    SKA_STRING_HASH_MAP_FOR_EACH(map, iter) {
        TEST_ASSERT_TRUE(strcmp(iter.key, "b") != 0);
        TEST_ASSERT_TRUE(strcmp(iter.key, "d") != 0);
        seen++;
    }
    TEST_ASSERT_EQUAL_INT32(2, (i32)seen);

    ska_string_hash_map_deinit(map);
}

// ===== Grow/shrink at scale =====

void seika_hash_map_string_grow_and_shrink_at_scale_test(void) {
    SkaStringHashMap* map = ska_string_hash_map_create_default_capacity();

    enum { N = 1500 };
    char key_buf[32];
    for (i32 i = 0; i < N; i++) {
        snprintf(key_buf, sizeof(key_buf), "key_%d", i);
        ska_string_hash_map_add(map, key_buf, &i, sizeof(i32));
    }
    TEST_ASSERT_EQUAL_INT32(N, (i32)map->size);
    TEST_ASSERT_TRUE(map->capacity >= map->size);

    for (i32 i = 0; i < N; i++) {
        snprintf(key_buf, sizeof(key_buf), "key_%d", i);
        i32* v = (i32*)ska_string_hash_map_get(map, key_buf);
        TEST_ASSERT_NOT_NULL(v);
        TEST_ASSERT_EQUAL_INT32(i, *v);
    }

    // Erase most of them back down -- should trigger shrink and stay correct.
    for (i32 i = 0; i < N - 10; i++) {
        snprintf(key_buf, sizeof(key_buf), "key_%d", i);
        TEST_ASSERT_TRUE(ska_string_hash_map_erase(map, key_buf));
    }
    TEST_ASSERT_EQUAL_INT32(10, (i32)map->size);

    for (i32 i = N - 10; i < N; i++) {
        snprintf(key_buf, sizeof(key_buf), "key_%d", i);
        i32* v = (i32*)ska_string_hash_map_get(map, key_buf);
        TEST_ASSERT_NOT_NULL(v);
        TEST_ASSERT_EQUAL_INT32(i, *v);
    }

    ska_string_hash_map_deinit(map);
}

// ===== Inline/heap boundary edge cases (key and value exactly at cap, one under, one over) =====

void seika_hash_map_string_key_boundary_test(void) {
    SkaStringHashMap* map = ska_string_hash_map_create_default_capacity();

    char key_under[SKA_STRING_HASH_MAP_INLINE_KEY_CAP - 2];
    memset(key_under, 'k', sizeof(key_under) - 1);
    key_under[sizeof(key_under) - 1] = '\0';

    char key_at[SKA_STRING_HASH_MAP_INLINE_KEY_CAP - 1];
    memset(key_at, 'm', sizeof(key_at) - 1);
    key_at[sizeof(key_at) - 1] = '\0';

    char key_over[SKA_STRING_HASH_MAP_INLINE_KEY_CAP + 16];
    memset(key_over, 'o', sizeof(key_over) - 1);
    key_over[sizeof(key_over) - 1] = '\0';

    i32 v1 = 1, v2 = 2, v3 = 3;
    ska_string_hash_map_add(map, key_under, &v1, sizeof(i32));
    ska_string_hash_map_add(map, key_at, &v2, sizeof(i32));
    ska_string_hash_map_add(map, key_over, &v3, sizeof(i32));

    TEST_ASSERT_EQUAL_INT32(1, *(i32*)ska_string_hash_map_get(map, key_under));
    TEST_ASSERT_EQUAL_INT32(2, *(i32*)ska_string_hash_map_get(map, key_at));
    TEST_ASSERT_EQUAL_INT32(3, *(i32*)ska_string_hash_map_get(map, key_over));

    ska_string_hash_map_deinit(map);
}

void seika_hash_map_string_value_boundary_test(void) {
    SkaStringHashMap* map = ska_string_hash_map_create_default_capacity();

    char val_under[SKA_STRING_HASH_MAP_INLINE_VALUE_CAP - 1];
    memset(val_under, 'a', sizeof(val_under) - 1);
    val_under[sizeof(val_under) - 1] = '\0';

    char val_at[SKA_STRING_HASH_MAP_INLINE_VALUE_CAP];
    memset(val_at, 'b', sizeof(val_at) - 1);
    val_at[sizeof(val_at) - 1] = '\0';

    char val_over[SKA_STRING_HASH_MAP_INLINE_VALUE_CAP + 32];
    memset(val_over, 'c', sizeof(val_over) - 1);
    val_over[sizeof(val_over) - 1] = '\0';

    ska_string_hash_map_add(map, "under", val_under, sizeof(val_under));
    ska_string_hash_map_add(map, "at", val_at, sizeof(val_at));
    ska_string_hash_map_add(map, "over", val_over, sizeof(val_over));

    TEST_ASSERT_EQUAL_STRING(val_under, (char*)ska_string_hash_map_get(map, "under"));
    TEST_ASSERT_EQUAL_STRING(val_at, (char*)ska_string_hash_map_get(map, "at"));
    TEST_ASSERT_EQUAL_STRING(val_over, (char*)ska_string_hash_map_get(map, "over"));

    ska_string_hash_map_deinit(map);
}

// ===== Representative real-world shapes =====

// Shaped like SkaCmdLineArgDef (~88 bytes on x64: 2 ptrs + bool + const char*[8]) -- the
// largest real fixed-size consumer value, deliberately near the inline cap.
typedef struct NearInlineCapPayload {
    void* ptr_a;
    void* ptr_b;
    bool flag;
    const char* names[8];
} NearInlineCapPayload;

void seika_hash_map_string_near_inline_cap_struct_test(void) {
    SkaStringHashMap* map = ska_string_hash_map_create_default_capacity();

    NearInlineCapPayload payload = { .ptr_a = (void*)1, .ptr_b = (void*)2, .flag = true, .names = {0} };
    payload.names[0] = "first";
    payload.names[1] = "second";

    ska_string_hash_map_add(map, "payload", &payload, sizeof(payload));
    NearInlineCapPayload* got = (NearInlineCapPayload*)ska_string_hash_map_get(map, "payload");
    TEST_ASSERT_NOT_NULL(got);
    TEST_ASSERT_EQUAL_PTR((void*)1, got->ptr_a);
    TEST_ASSERT_EQUAL_PTR((void*)2, got->ptr_b);
    TEST_ASSERT_TRUE(got->flag);
    TEST_ASSERT_EQUAL_STRING("first", got->names[0]);
    TEST_ASSERT_EQUAL_STRING("second", got->names[1]);

    ska_string_hash_map_deinit(map);
}

// Shaped like templ's TplContext usage: unbounded-length string values (heap-fallback path).
void seika_hash_map_string_unbounded_string_value_test(void) {
    SkaStringHashMap* map = ska_string_hash_map_create_default_capacity();

    const char* long_value =
        "this rendered template variable content is deliberately much longer than any "
        "fixed-size inline value buffer would comfortably hold, exercising the heap-fallback "
        "path end to end, the same way templ's TplContext does for real page content";

    ska_string_hash_map_add(map, "page_content", long_value, strlen(long_value) + 1);
    const char* got = (const char*)ska_string_hash_map_get(map, "page_content");
    TEST_ASSERT_NOT_NULL(got);
    TEST_ASSERT_EQUAL_STRING(long_value, got);

    ska_string_hash_map_deinit(map);
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(seika_hash_map_string_basic_roundtrip_test);
    RUN_TEST(seika_hash_map_string_typed_sugar_test);
    RUN_TEST(seika_hash_map_string_duplicate_add_does_not_update_test);
    RUN_TEST(seika_hash_map_string_set_test);
    RUN_TEST(seika_hash_map_string_set_across_inline_boundary_test);

    RUN_TEST(seika_hash_map_string_iterator_full_traversal_test);
    RUN_TEST(seika_hash_map_string_iterator_skips_tombstones_test);

    RUN_TEST(seika_hash_map_string_grow_and_shrink_at_scale_test);

    RUN_TEST(seika_hash_map_string_key_boundary_test);
    RUN_TEST(seika_hash_map_string_value_boundary_test);

    RUN_TEST(seika_hash_map_string_near_inline_cap_struct_test);
    RUN_TEST(seika_hash_map_string_unbounded_string_value_test);

    return UNITY_END();
}
