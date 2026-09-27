#pragma once

#include "seika/assert.h"
#include "seika/memory.h"

#define SKA_HASH_MAP_T_SHRINK_THRESHOLD 0.25f
#define SKA_HASH_MAP_T_MIN_CAPACITY 8

// Forward declare the macros
#define SKA_HASH_MAP_DECLARE(STRUCT_NAME, KEY_TYPE, VALUE_TYPE) \
    SKA_HASH_MAP_DECLARE_INTERNAL(STRUCT_NAME, KEY_TYPE, VALUE_TYPE)

#define SKA_HASH_MAP_IMPLEMENT(STRUCT_NAME, KEY_TYPE, VALUE_TYPE) \
    SKA_HASH_MAP_IMPLEMENT_INTERNAL(STRUCT_NAME, KEY_TYPE, VALUE_TYPE)

// Internal declaration macro
#define SKA_HASH_MAP_DECLARE_INTERNAL(STRUCT_NAME, KEY_TYPE, VALUE_TYPE) \
    typedef usize (*STRUCT_NAME##HashFunc_t)(KEY_TYPE const*); \
    typedef int32 (*STRUCT_NAME##CompareFunc_t)(KEY_TYPE const*, KEY_TYPE const*); \
    \
    typedef struct STRUCT_NAME##Node { \
        struct STRUCT_NAME##Node* next; \
        KEY_TYPE key; \
        VALUE_TYPE value; \
    } STRUCT_NAME##Node; \
    \
    typedef struct STRUCT_NAME { \
        SkaAllocator allocator; \
        usize capacity; \
        usize initialCapacity; \
        usize size; \
        STRUCT_NAME##HashFunc_t hashFunc; \
        STRUCT_NAME##CompareFunc_t compareFunc; \
        STRUCT_NAME##Node** nodes; \
    } STRUCT_NAME; \
    \
    typedef struct STRUCT_NAME##Iterator { \
        usize count; \
        usize end; \
        usize index; \
        STRUCT_NAME##Node* pair; \
    } STRUCT_NAME##Iterator; \
    \
    typedef struct STRUCT_NAME##AddResult { \
        usize hash; \
        bool was_added; \
    } STRUCT_NAME##AddResult; \
    \
    STRUCT_NAME* STRUCT_NAME##_init(usize capacity); \
    STRUCT_NAME* STRUCT_NAME##_init_a(SkaAllocator allocator, usize capacity); \
    bool STRUCT_NAME##_deinit(STRUCT_NAME* hashMap); \
    STRUCT_NAME##AddResult STRUCT_NAME##_add(STRUCT_NAME* hashMap, KEY_TYPE key, VALUE_TYPE value); \
    VALUE_TYPE* STRUCT_NAME##_get(STRUCT_NAME* hashMap, KEY_TYPE key); \
    VALUE_TYPE* STRUCT_NAME##_get_with_hash(STRUCT_NAME* hashMap, KEY_TYPE key, usize hash); \
    bool STRUCT_NAME##_has(STRUCT_NAME* hashMap, KEY_TYPE key); \
    bool STRUCT_NAME##_erase(STRUCT_NAME* hashMap, KEY_TYPE key); \
    void STRUCT_NAME##_clear(STRUCT_NAME* hashMap); \
    STRUCT_NAME##Iterator STRUCT_NAME##_iter_create(STRUCT_NAME* hashMap); \
    bool STRUCT_NAME##_iter_is_valid(STRUCT_NAME* hashMap, STRUCT_NAME##Iterator* iterator); \
    void STRUCT_NAME##_iter_advance(STRUCT_NAME* hashMap, STRUCT_NAME##Iterator* iterator);

// Internal implementation macro
#define SKA_HASH_MAP_IMPLEMENT_INTERNAL(STRUCT_NAME, KEY_TYPE, VALUE_TYPE) \
    static usize STRUCT_NAME##_default_hash(KEY_TYPE const* key) { \
        usize hash = 5381; \
        const unsigned char* bytes = (const unsigned char*)key; \
        for (usize i = 0; i < sizeof(KEY_TYPE); i++) { \
            hash = ((hash << 5) + hash) ^ bytes[i]; \
        } \
        return hash; \
    } \
    \
    static int32 STRUCT_NAME##_default_compare(KEY_TYPE const* a, KEY_TYPE const* b) { \
        return memcmp(a, b, sizeof(KEY_TYPE)); \
    } \
    \
    static STRUCT_NAME##Node* STRUCT_NAME##_create_node( \
        STRUCT_NAME* hashMap, KEY_TYPE key, VALUE_TYPE value, STRUCT_NAME##Node* next) { \
        STRUCT_NAME##Node* node = SKA_ALLOC_A(hashMap->allocator, STRUCT_NAME##Node); \
        node->key = key; \
        node->value = value; \
        node->next = next; \
        return node; \
    } \
    \
    static void STRUCT_NAME##_destroy_node(STRUCT_NAME* map, STRUCT_NAME##Node* node) { \
        SKA_ASSERT(node != NULL); \
        SKA_FREE_A(map->allocator, node); \
    } \
    \
    static bool STRUCT_NAME##_push_front( \
        STRUCT_NAME* hashMap, usize index, KEY_TYPE key, VALUE_TYPE value) { \
        hashMap->nodes[index] = STRUCT_NAME##_create_node(hashMap, key, value, hashMap->nodes[index]); \
        return hashMap->nodes[index] != NULL; \
    } \
    \
    static void STRUCT_NAME##_allocate(STRUCT_NAME* hashMap, usize capacity) { \
        hashMap->nodes = (STRUCT_NAME##Node**)SKA_ALLOC_BYTES_A( \
            hashMap->allocator, capacity * sizeof(STRUCT_NAME##Node*)); \
        memset(hashMap->nodes, 0, capacity * sizeof(STRUCT_NAME##Node*)); \
        hashMap->capacity = capacity; \
    } \
    \
    static void STRUCT_NAME##_rehash( \
        STRUCT_NAME* hashMap, STRUCT_NAME##Node** oldNode, usize oldCapacity) { \
        for (usize chain = 0; chain < oldCapacity; chain++) { \
            for (STRUCT_NAME##Node* node = oldNode[chain]; node != NULL;) { \
                STRUCT_NAME##Node* next = node->next; \
                usize newIndex = hashMap->hashFunc((KEY_TYPE const*)&node->key) % hashMap->capacity; \
                node->next = hashMap->nodes[newIndex]; \
                hashMap->nodes[newIndex] = node; \
                node = next; \
            } \
        } \
    } \
    \
    static void STRUCT_NAME##_resize(STRUCT_NAME* hashMap, usize capacity) { \
        if (capacity < SKA_HASH_MAP_T_MIN_CAPACITY) { \
            if (hashMap->capacity > SKA_HASH_MAP_T_MIN_CAPACITY) { \
                capacity = SKA_HASH_MAP_T_MIN_CAPACITY; \
            } else { \
                return; \
            } \
        } \
        STRUCT_NAME##Node** oldNode = hashMap->nodes; \
        usize oldCapacity = hashMap->capacity; \
        STRUCT_NAME##_allocate(hashMap, capacity); \
        STRUCT_NAME##_rehash(hashMap, oldNode, oldCapacity); \
        SKA_FREE_A(hashMap->allocator, oldNode); \
    } \
    \
    static void STRUCT_NAME##_grow_if_needed(STRUCT_NAME* hashMap) { \
        if (hashMap->size >= hashMap->capacity) { \
            STRUCT_NAME##_resize(hashMap, hashMap->size * 2); \
        } \
    } \
    \
    static void STRUCT_NAME##_shrink_if_needed(STRUCT_NAME* hashMap) { \
        const usize shrinkCapacity = (usize)((f32)hashMap->capacity * SKA_HASH_MAP_T_SHRINK_THRESHOLD); \
        if (hashMap->size <= shrinkCapacity && shrinkCapacity >= hashMap->initialCapacity) { \
            usize targetCapacity = shrinkCapacity > hashMap->initialCapacity ? shrinkCapacity : hashMap->initialCapacity; \
            STRUCT_NAME##_resize(hashMap, targetCapacity); \
        } \
    } \
    \
    STRUCT_NAME* STRUCT_NAME##_init(usize capacity) { \
        return STRUCT_NAME##_init_a(ska_mem_get_global_allocator(), capacity); \
    } \
    \
    STRUCT_NAME* STRUCT_NAME##_init_a(SkaAllocator allocator, usize capacity) { \
        STRUCT_NAME* map = SKA_ALLOC_A(allocator, STRUCT_NAME); \
        map->allocator = allocator; \
        map->size = 0; \
        map->initialCapacity = capacity; \
        map->hashFunc = STRUCT_NAME##_default_hash; \
        map->compareFunc = STRUCT_NAME##_default_compare; \
        STRUCT_NAME##_allocate(map, capacity); \
        return map; \
    } \
    \
    bool STRUCT_NAME##_deinit(STRUCT_NAME* hashMap) { \
        STRUCT_NAME##Node* node = NULL; \
        STRUCT_NAME##Node* next = NULL; \
        for (usize chain = 0; chain < hashMap->capacity; chain++) { \
            node = hashMap->nodes[chain]; \
            while (node != NULL) { \
                next = node->next; \
                STRUCT_NAME##_destroy_node(hashMap, node); \
                node = next; \
            } \
        } \
        SKA_FREE_A(hashMap->allocator, hashMap->nodes); \
        SKA_FREE_A(hashMap->allocator, hashMap); \
        return true; \
    } \
    \
    STRUCT_NAME##AddResult STRUCT_NAME##_add( \
        STRUCT_NAME* hashMap, KEY_TYPE key, VALUE_TYPE value) { \
        SKA_ASSERT(hashMap != NULL); \
        STRUCT_NAME##_grow_if_needed(hashMap); \
        const usize index = hashMap->hashFunc((KEY_TYPE const*)&key) % hashMap->capacity; \
        VALUE_TYPE* existingValue = STRUCT_NAME##_get_with_hash(hashMap, key, index); \
        if (existingValue) { \
            *existingValue = value; \
            return (STRUCT_NAME##AddResult){ .hash = index, .was_added = false }; \
        } \
        if (!STRUCT_NAME##_push_front(hashMap, index, key, value)) { \
            return (STRUCT_NAME##AddResult){ .hash = index, .was_added = false }; \
        } \
        hashMap->size++; \
        return (STRUCT_NAME##AddResult){ .hash = index, .was_added = true }; \
    } \
    \
    VALUE_TYPE* STRUCT_NAME##_get(STRUCT_NAME* hashMap, KEY_TYPE key) { \
        SKA_ASSERT(hashMap != NULL); \
        const usize index = hashMap->hashFunc((KEY_TYPE const*)&key) % hashMap->capacity; \
        for (STRUCT_NAME##Node* node = hashMap->nodes[index]; node; node = node->next) { \
            if (hashMap->compareFunc((KEY_TYPE const*)&key, (KEY_TYPE const*)&node->key) == 0) { \
                return &node->value; \
            } \
        } \
        return NULL; \
    } \
    \
    VALUE_TYPE* STRUCT_NAME##_get_with_hash( \
        STRUCT_NAME* hashMap, KEY_TYPE key, usize hash) { \
        SKA_ASSERT(hashMap != NULL); \
        for (STRUCT_NAME##Node* node = hashMap->nodes[hash]; node; node = node->next) { \
            if (hashMap->compareFunc((KEY_TYPE const*)&key, (KEY_TYPE const*)&node->key) == 0) { \
                return &node->value; \
            } \
        } \
        return NULL; \
    } \
    \
    bool STRUCT_NAME##_has(STRUCT_NAME* hashMap, KEY_TYPE key) { \
        return STRUCT_NAME##_get(hashMap, key) != NULL; \
    } \
    \
    bool STRUCT_NAME##_erase(STRUCT_NAME* hashMap, KEY_TYPE key) { \
        SKA_ASSERT(hashMap != NULL); \
        const usize index = hashMap->hashFunc((KEY_TYPE const*)&key) % hashMap->capacity; \
        STRUCT_NAME##Node* node = hashMap->nodes[index]; \
        for (STRUCT_NAME##Node* previous = NULL; node; previous = node, node = node->next) { \
            if (hashMap->compareFunc((KEY_TYPE const*)&key, (KEY_TYPE const*)&node->key) == 0) { \
                if (previous != NULL) { \
                    previous->next = node->next; \
                } else { \
                    hashMap->nodes[index] = node->next; \
                } \
                STRUCT_NAME##_destroy_node(hashMap, node); \
                hashMap->size--; \
                STRUCT_NAME##_shrink_if_needed(hashMap); \
                return true; \
            } \
        } \
        return false; \
    } \
    \
    void STRUCT_NAME##_clear(STRUCT_NAME* hashMap) { \
        SKA_ASSERT(hashMap != NULL); \
        for (usize chain = 0; chain < hashMap->capacity; chain++) { \
            STRUCT_NAME##Node* node = hashMap->nodes[chain]; \
            while (node != NULL) { \
                STRUCT_NAME##Node* next = node->next; \
                STRUCT_NAME##_destroy_node(hashMap, node); \
                node = next; \
            } \
            hashMap->nodes[chain] = NULL; \
        } \
        hashMap->size = 0; \
    } \
    \
    STRUCT_NAME##Iterator STRUCT_NAME##_iter_create(STRUCT_NAME* hashMap) { \
        STRUCT_NAME##Node* initialNode = NULL; \
        usize initialIndex = 0; \
        for (usize chain = 0; chain < hashMap->capacity; chain++) { \
            STRUCT_NAME##Node* node = hashMap->nodes[chain]; \
            if (node != NULL) { \
                initialNode = node; \
                initialIndex = chain + 1; \
                break; \
            } \
        } \
        usize iteratorCount = initialNode != NULL ? 1 : 0; \
        STRUCT_NAME##Iterator iterator = { \
            .count = iteratorCount, \
            .end = hashMap->capacity, \
            .index = initialIndex, \
            .pair = initialNode \
        }; \
        return iterator; \
    } \
    \
    bool STRUCT_NAME##_iter_is_valid( \
        STRUCT_NAME* hashMap, STRUCT_NAME##Iterator* iterator) { \
        return iterator->pair != NULL && iterator->count <= hashMap->size; \
    } \
    \
    void STRUCT_NAME##_iter_advance( \
        STRUCT_NAME* hashMap, STRUCT_NAME##Iterator* iterator) { \
        if (STRUCT_NAME##_iter_is_valid(hashMap, iterator)) { \
            if (iterator->pair->next != NULL) { \
                iterator->pair = iterator->pair->next; \
                iterator->count++; \
                return; \
            } \
            for (usize chain = iterator->index; chain < hashMap->capacity; chain++) { \
                STRUCT_NAME##Node* node = hashMap->nodes[chain]; \
                if (node != NULL) { \
                    iterator->pair = node; \
                    iterator->index = chain + 1; \
                    iterator->count++; \
                    return; \
                } \
            } \
        } \
        iterator->pair = NULL; \
    }

// Convenience for-each macro
#define SKA_HASH_MAP_T_FOR_EACH(HASH_MAP, STRUCT_NAME, ITER) \
    for (STRUCT_NAME##Iterator ITER = STRUCT_NAME##_iter_create(HASH_MAP); \
         STRUCT_NAME##_iter_is_valid(HASH_MAP, &(ITER)); \
         STRUCT_NAME##_iter_advance(HASH_MAP, &(ITER)))
