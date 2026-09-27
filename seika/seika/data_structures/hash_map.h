#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "seika/defines.h"
#include "seika/memory.h"

#define SKA_HASH_MAP_SHRINK_THRESHOLD 0.25f
#define SKA_HASH_MAP_MIN_CAPACITY 8

#define SKA_HASH_MAP_FOR_EACH(HASH_MAP, ITER_NAME) \
for (SkaHashMapIterator ITER_NAME = ska_hash_map_iter_create(HASH_MAP); ska_hash_map_iter_is_valid(HASH_MAP, &(ITER_NAME)); ska_hash_map_iter_advance(HASH_MAP, &(ITER_NAME)))

typedef usize (*SkaHashFunc) (void*, usize);
typedef int32 (*SkaCompareFunc) (void*, void*, usize);

typedef struct SkaHashMapNode {
    struct SkaHashMapNode* next;
    void* key;
    void* value;
} SkaHashMapNode;

typedef struct SkaHashMap {
    SkaAllocator allocator;
    usize key_size;
    usize value_size;
    usize capacity;
    usize initial_capacity;
    usize size;
    SkaHashFunc hash_func;
    SkaCompareFunc compare_func;
    SkaHashMapNode** nodes;
} SkaHashMap;

typedef struct SkaHashMapIterator {
    usize count;
    usize end;
    usize index;
    SkaHashMapNode* pair;
} SkaHashMapIterator;

typedef struct SkaHashMapAddResult {
    usize hash;
    bool was_added;
} SkaHashMapAddResult;

// Generic hash map methods
SkaHashMap* ska_hash_map_init(usize key_size, usize value_size, usize capacity);
SkaHashMap* ska_hash_map_init_a(SkaAllocator allocator, usize key_size, usize value_size, usize capacity);
bool ska_hash_map_deinit(SkaHashMap* hash_map);
SkaHashMapAddResult ska_hash_map_add(SkaHashMap* hash_map, const void* key, void* value);
void* ska_hash_map_get(const SkaHashMap* hash_map, const void* key);
// Same as ska_hash_map_get but doesn't execute the hash function to get the value
void* ska_hash_map_get_with_hash(const SkaHashMap* hash_map, const void* key, usize hash);
bool ska_hash_map_has(const SkaHashMap* hash_map, const void* key);
bool ska_hash_map_erase(SkaHashMap* hash_map, const void* key);
void ska_hash_map_clear(SkaHashMap* hash_map);

// Iterator
SkaHashMapIterator ska_hash_map_iter_create(SkaHashMap* hash_map);
bool ska_hash_map_iter_is_valid(SkaHashMap* hash_map, SkaHashMapIterator* iterator);
void ska_hash_map_iter_advance(SkaHashMap* hash_map, SkaHashMapIterator* iterator);

#ifdef __cplusplus
}
#endif
