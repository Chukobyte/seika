#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdbool.h>

#include "seika/defines.h"

#define SKA_STRING_HASH_MAP_SHRINK_THRESHOLD 0.25f
#define SKA_STRING_HASH_MAP_MIN_CAPACITY 8

#define SKA_STRING_HASH_MAP_FOR_EACH(HASH_MAP, ITER_NAME) \
for (SkaStringHashMapIterator ITER_NAME = ska_string_hash_map_iter_create(HASH_MAP); ska_string_hash_map_iter_is_valid(HASH_MAP, &(ITER_NAME)); ska_string_hash_map_iter_advance(HASH_MAP, &(ITER_NAME)))

typedef size_t (*SkaStringHashFunc) (const char*);
typedef int32 (*SkaStringCompareFunc) (const char*, const char*);

typedef struct SkaStringHashMapNode {
    struct SkaStringHashMapNode* next;
    char* key;
    void* value;
    size_t valueSize;
} SkaStringHashMapNode;

typedef struct SkaStringHashMap {
    size_t capacity;
    size_t size;
    SkaStringHashFunc hashFunc;
    SkaStringCompareFunc compareFunc;
    SkaStringHashMapNode** nodes;
} SkaStringHashMap;

typedef struct SkaStringHashMapIterator {
    size_t count;
    size_t end;
    size_t index;
    SkaStringHashMapNode* pair;
} SkaStringHashMapIterator;

// Generic hash map methods
SkaStringHashMap* ska_string_hash_map_create(size_t capacity);
SkaStringHashMap* ska_string_hash_map_create_default_capacity();
bool ska_string_hash_map_destroy(SkaStringHashMap* hashMap);
bool ska_string_hash_map_add(SkaStringHashMap* hashMap, const char* key, const void* value, size_t valueSize);
void* ska_string_hash_map_get(SkaStringHashMap* hashMap, const char* key);
void* ska_string_hash_map_find(SkaStringHashMap* hashMap, const char* key);
bool ska_string_hash_map_has(SkaStringHashMap* hashMap, const char* key);
bool ska_string_hash_map_erase(SkaStringHashMap* hashMap, const char* key);
// Int
bool ska_string_hash_map_add_int(SkaStringHashMap* hashMap, const char* key, int value);
int ska_string_hash_map_get_int(SkaStringHashMap* hashMap, const char* key);
// String
bool ska_string_hash_map_add_string(SkaStringHashMap* hashMap, const char* key, const char* value);
char* ska_string_hash_map_get_string(SkaStringHashMap* hashMap, const char* key);

// Iterator
SkaStringHashMapIterator ska_string_hash_map_iter_create(SkaStringHashMap* hashMap);
bool ska_string_hash_map_iter_is_valid(SkaStringHashMap* hashMap, SkaStringHashMapIterator* iterator);
void ska_string_hash_map_iter_advance(SkaStringHashMap* hashMap, SkaStringHashMapIterator* iterator);

#ifdef __cplusplus
}
#endif
