#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "../defines.h"
#include "seika/memory/memory.h"

#define SKA_STRING_HASH_MAP_SHRINK_THRESHOLD 0.25f
#define SKA_STRING_HASH_MAP_MIN_CAPACITY 8

#define SKA_STRING_HASH_MAP_FOR_EACH(HASH_MAP, ITER_NAME) \
for (SkaStringHashMapIterator ITER_NAME = ska_string_hash_map_iter_create(HASH_MAP); ska_string_hash_map_iter_is_valid(HASH_MAP, &(ITER_NAME)); ska_string_hash_map_iter_advance(HASH_MAP, &(ITER_NAME)))

typedef usize (*SkaStringHashFunc) (const char*);
typedef int32 (*SkaStringCompareFunc) (const char*, const char*);

typedef struct SkaStringHashMapNode {
    struct SkaStringHashMapNode* next;
    char* key;
    void* value;
    usize valueSize;
} SkaStringHashMapNode;

typedef struct SkaStringHashMap {
    SkaAllocator allocator;
    usize capacity;
    usize initialCapacity;
    usize size;
    SkaStringHashFunc hashFunc;
    SkaStringCompareFunc compareFunc;
    SkaStringHashMapNode** nodes;
} SkaStringHashMap;

typedef struct SkaStringHashMapIterator {
    usize count;
    usize end;
    usize index;
    SkaStringHashMapNode* pair;
} SkaStringHashMapIterator;

typedef struct SkaStringHashMapAddResult {
    void* value;
    usize hash;
    bool wasAdded;
} SkaStringHashMapAddResult;

// Generic hash map methods
SkaStringHashMap* ska_string_hash_map_init(usize capacity);
SkaStringHashMap* ska_string_hash_map_init_a(SkaAllocator allocator, usize capacity);
SkaStringHashMap* ska_string_hash_map_create_default_capacity();
bool ska_string_hash_map_deinit(SkaStringHashMap* hashMap);
SkaStringHashMapAddResult ska_string_hash_map_add(SkaStringHashMap* hashMap, const char* key, const void* value, usize valueSize);
void* ska_string_hash_map_get(const SkaStringHashMap* hashMap, const char* key);
void* ska_string_hash_map_get_with_hash(const SkaStringHashMap* hashMap, const char* key, usize hash);
bool ska_string_hash_map_has(SkaStringHashMap* hashMap, const char* key);
bool ska_string_hash_map_erase(SkaStringHashMap* hashMap, const char* key);
// Int
SkaStringHashMapAddResult ska_string_hash_map_add_int(SkaStringHashMap* hashMap, const char* key, int value);
int ska_string_hash_map_get_int(SkaStringHashMap* hashMap, const char* key);
// String
SkaStringHashMapAddResult ska_string_hash_map_add_string(SkaStringHashMap* hashMap, const char* key, const char* value);
char* ska_string_hash_map_get_string(SkaStringHashMap* hashMap, const char* key);

// Iterator
SkaStringHashMapIterator ska_string_hash_map_iter_create(SkaStringHashMap* hashMap);
bool ska_string_hash_map_iter_is_valid(SkaStringHashMap* hashMap, SkaStringHashMapIterator* iterator);
void ska_string_hash_map_iter_advance(SkaStringHashMap* hashMap, SkaStringHashMapIterator* iterator);

#ifdef __cplusplus
}
#endif
