#pragma once

#include "seika/assert.h"
#include "seika/memory.h"

// Forward declare the macros
#define SKA_ARRAY_LIST_DECLARE(STRUCT_NAME, TYPE) \
    SKA_ARRAY_LIST_DECLARE_INTERNAL(STRUCT_NAME, TYPE)

#define SKA_ARRAY_LIST_IMPLEMENT(STRUCT_NAME, TYPE) \
    SKA_ARRAY_LIST_IMPLEMENT_INTERNAL(STRUCT_NAME, TYPE)

// Internal declaration macro
#define SKA_ARRAY_LIST_DECLARE_INTERNAL(STRUCT_NAME, TYPE) \
    typedef bool (*STRUCT_NAME##Cmp_t)(TYPE const*, TYPE const*); \
    \
    typedef struct STRUCT_NAME { \
        SkaAllocator allocator; \
        TYPE* data; \
        usize size; \
        usize capacity; \
        usize initialCapacity; \
    } STRUCT_NAME; \
    \
    STRUCT_NAME STRUCT_NAME##_init_default_capacity(void); \
    STRUCT_NAME STRUCT_NAME##_init(usize initialCapacity); \
    STRUCT_NAME STRUCT_NAME##_init_a(SkaAllocator allocator, usize initialCapacity); \
    void STRUCT_NAME##_deinit(STRUCT_NAME* list); \
    bool STRUCT_NAME##_append(STRUCT_NAME* list, TYPE value); \
    TYPE* STRUCT_NAME##_get(STRUCT_NAME* list, usize index); \
    TYPE const* STRUCT_NAME##_get_const(const STRUCT_NAME* list, usize index); \
    bool STRUCT_NAME##_remove(STRUCT_NAME* list, TYPE value); \
    bool STRUCT_NAME##_remove2(STRUCT_NAME* list, TYPE value, STRUCT_NAME##Cmp_t compareFunc); \
    bool STRUCT_NAME##_remove_by_index(STRUCT_NAME* list, usize index); \
    bool STRUCT_NAME##_has(const STRUCT_NAME* list, TYPE value); \
    bool STRUCT_NAME##_has2(const STRUCT_NAME* list, TYPE value, STRUCT_NAME##Cmp_t compareFunc); \
    bool STRUCT_NAME##_is_empty(const STRUCT_NAME* list); \
    void STRUCT_NAME##_clear(STRUCT_NAME* list); \
    void STRUCT_NAME##_reserve(STRUCT_NAME* list, usize newCapacity); \
    TYPE* STRUCT_NAME##_to_array(STRUCT_NAME* list);

// Internal implementation macro
#define SKA_ARRAY_LIST_IMPLEMENT_INTERNAL(STRUCT_NAME, TYPE) \
    static bool STRUCT_NAME##_default_compare(TYPE const* a, TYPE const* b) { \
        return memcmp(a, b, sizeof(TYPE)) == 0; \
    } \
    \
    STRUCT_NAME STRUCT_NAME##_init_default_capacity(void) { \
        return STRUCT_NAME##_init(16); \
    } \
    \
    STRUCT_NAME STRUCT_NAME##_init(usize initialCapacity) { \
        return STRUCT_NAME##_init_a(ska_mem_get_global_allocator(), initialCapacity); \
    } \
    \
    STRUCT_NAME STRUCT_NAME##_init_a(SkaAllocator allocator, usize initialCapacity) { \
        return (STRUCT_NAME){ \
            .allocator = allocator, \
            .data = (TYPE*)SKA_ALLOC_BYTES_ZEROED_A(allocator, initialCapacity * sizeof(TYPE)), \
            .capacity = initialCapacity, \
            .initialCapacity = initialCapacity, \
            .size = 0 \
        }; \
    } \
    \
    void STRUCT_NAME##_deinit(STRUCT_NAME* list) { \
        SKA_FREE_A(list->allocator, list->data); \
        *list = (STRUCT_NAME){0}; \
    } \
    \
    bool STRUCT_NAME##_append(STRUCT_NAME* list, TYPE value) { \
        if (list->size >= list->capacity) { \
            const usize newCapacity = (usize)((f32)list->capacity * 1.5f); \
            list->capacity = newCapacity; \
            list->data = (TYPE*)SKA_REALLOC_A(list->allocator, list->data, list->capacity * sizeof(TYPE)); \
            if (list->data == NULL) return false; \
        } \
        list->data[list->size] = value; \
        list->size++; \
        return true; \
    } \
    \
    TYPE* STRUCT_NAME##_get(STRUCT_NAME* list, usize index) { \
        SKA_ASSERT_FMT(index < list->size, "Attempting to access out of bounds index '%zu'", index); \
        return &list->data[index]; \
    } \
    \
    TYPE const* STRUCT_NAME##_get_const(const STRUCT_NAME* list, usize index) { \
        SKA_ASSERT_FMT(index < list->size, "Attempting to access out of bounds index '%zu'", index); \
        return &list->data[index]; \
    } \
    \
    bool STRUCT_NAME##_remove(STRUCT_NAME* list, TYPE value) { \
        return STRUCT_NAME##_remove2(list, value, STRUCT_NAME##_default_compare); \
    } \
    \
    bool STRUCT_NAME##_remove2(STRUCT_NAME* list, TYPE value, STRUCT_NAME##Cmp_t compareFunc) { \
        for (usize index = 0; index < list->size; index++) { \
            if (compareFunc((TYPE const*)&value, (TYPE const*)&list->data[index])) { \
                for (usize i = index + 1; i < list->size; ++i) { \
                    list->data[i - 1] = list->data[i]; \
                } \
                list->size--; \
                return true; \
            } \
        } \
        return false; \
    } \
    \
    bool STRUCT_NAME##_remove_by_index(STRUCT_NAME* list, usize index) { \
        if (index < list->size) { \
            for (usize i = index + 1; i < list->size; ++i) { \
                list->data[i - 1] = list->data[i]; \
            } \
            list->size--; \
            return true; \
        } \
        return false; \
    } \
    \
    bool STRUCT_NAME##_has(const STRUCT_NAME* list, TYPE value) { \
        return STRUCT_NAME##_has2(list, value, STRUCT_NAME##_default_compare); \
    } \
    \
    bool STRUCT_NAME##_has2(const STRUCT_NAME* list, TYPE value, STRUCT_NAME##Cmp_t compareFunc) { \
        for (usize i = 0; i < list->size; i++) { \
            if (compareFunc((TYPE const*)&value, (TYPE const*)&list->data[i])) { \
                return true; \
            } \
        } \
        return false; \
    } \
    \
    bool STRUCT_NAME##_is_empty(const STRUCT_NAME* list) { \
        return list->size == 0; \
    } \
    \
    void STRUCT_NAME##_clear(STRUCT_NAME* list) { \
        list->size = 0; \
    } \
    \
    void STRUCT_NAME##_reserve(STRUCT_NAME* list, usize newCapacity) { \
        if (newCapacity > list->capacity) { \
            list->data = (TYPE*)SKA_REALLOC_A(list->allocator, list->data, newCapacity * sizeof(TYPE)); \
            list->capacity = newCapacity; \
        } \
    } \
    \
    TYPE* STRUCT_NAME##_to_array(STRUCT_NAME* list) { \
        return list->data; \
    }

// Convenience for-each macro
#define SKA_ARRAY_LIST_T_FOR_EACH(LIST, TYPE, ITER) \
    for(TYPE* ITER = (LIST)->data; ITER < (LIST)->data + (LIST)->size; ITER++)
