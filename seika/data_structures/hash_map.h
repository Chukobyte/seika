#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdbool.h>

#include "seika/defines.h"

#define SKA_HASH_MAP_SHRINK_THRESHOLD 0.25f
#define SKA_HASH_MAP_MIN_CAPACITY 8

#define SKA_HASH_MAP_FOR_EACH(HASH_MAP, ITER_NAME) \
for (SkaHashMapIterator ITER_NAME = ska_hash_map_iter_create(HASH_MAP); ska_hash_map_iter_is_valid(HASH_MAP, &(ITER_NAME)); ska_hash_map_iter_advance(HASH_MAP, &(ITER_NAME)))

typedef size_t (*SkaHashFunc) (void*, size_t);
typedef int32 (*SkaCompareFunc) (void*, void*, size_t);

typedef struct SkaHashMapNode {
    struct SkaHashMapNode* next;
    void* key;
    void* value;
} SkaHashMapNode;

typedef struct SkaHashMap {
    size_t keySize;
    size_t valueSize;
    size_t capacity;
    size_t size;
    SkaHashFunc hashFunc;
    SkaCompareFunc compareFunc;
    SkaHashMapNode** nodes;
} SkaHashMap;

typedef struct SkaHashMapIterator {
    size_t count;
    size_t end;
    size_t index;
    SkaHashMapNode* pair;
} SkaHashMapIterator;

// Generic hash map methods
SkaHashMap* ska_hash_map_create(size_t keySize, size_t valueSize, size_t capacity);
bool ska_hash_map_destroy(SkaHashMap* hashMap);
bool ska_hash_map_add(SkaHashMap* hashMap, void* key, void* value);
void* ska_hash_map_get(SkaHashMap* hashMap, void* key);
bool ska_hash_map_has(SkaHashMap* hashMap, void* key);
bool ska_hash_map_erase(SkaHashMap* hashMap, void* key);

// Iterator
SkaHashMapIterator ska_hash_map_iter_create(SkaHashMap* hashMap);
bool ska_hash_map_iter_is_valid(SkaHashMap* hashMap, SkaHashMapIterator* iterator);
void ska_hash_map_iter_advance(SkaHashMap* hashMap, SkaHashMapIterator* iterator);

#ifdef __cplusplus
}
#endif
