#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "../defines.h"
#include "seika/memory/memory.h"

#define SKA_STRING_HASH_MAP_SHRINK_THRESHOLD 0.25f
#define SKA_STRING_HASH_MAP_MIN_CAPACITY 8

// Small-buffer-inline sizing: covers every known fixed-size consumer and observed key with
// headroom, falling back to one heap allocation only when a payload exceeds this (in practice,
// only templ's TplContext, whose values are arbitrary-length rendered template content).
#define SKA_STRING_HASH_MAP_INLINE_KEY_CAP 48
#define SKA_STRING_HASH_MAP_INLINE_VALUE_CAP 96

#define SKA_STRING_HASH_MAP_FOR_EACH(HASH_MAP, ITER_NAME) \
for (SkaStringHashMapIterator ITER_NAME = ska_string_hash_map_iter_create(HASH_MAP); ska_string_hash_map_iter_is_valid(HASH_MAP, &(ITER_NAME)); ska_string_hash_map_iter_advance(HASH_MAP, &(ITER_NAME)))

typedef usize (*SkaStringHashFunc) (const char*);
typedef int32 (*SkaStringCompareFunc) (const char*, const char*);

// Open-addressing (linear probing, power-of-2 capacity, tombstone deletion) slot. Small
// keys/values live inline (zero allocation); anything larger falls back to one heap allocation
// for that field only. A plain struct-copy (resize/rehash) correctly carries both along.
typedef struct SkaStringHashMapSlot {
    usize hash;       // cached -- avoids recomputing hash_func on rehash/resize
    bool occupied;
    bool tombstone;    // true iff this slot held an entry that was erased (keep probing past it)
    usize key_len;      // excludes NUL
    char key_inline[SKA_STRING_HASH_MAP_INLINE_KEY_CAP];
    char* key_heap;      // NULL unless key_len >= SKA_STRING_HASH_MAP_INLINE_KEY_CAP
    usize value_size;
    char value_inline[SKA_STRING_HASH_MAP_INLINE_VALUE_CAP];
    void* value_heap;     // NULL unless value_size > SKA_STRING_HASH_MAP_INLINE_VALUE_CAP
} SkaStringHashMapSlot;

typedef struct SkaStringHashMap {
    SkaAllocator allocator;
    SkaStringHashMapSlot* slots;
    usize capacity;          // always a power of 2
    usize initial_capacity;
    usize size;               // live (occupied, non-tombstone) entry count
    usize tombstone_count;
    SkaStringHashFunc hash_func;
    SkaStringCompareFunc compare_func;
} SkaStringHashMap;

// .key/.value are populated from the current entry on create/advance -- callers read these
// directly rather than reaching into slot internals (the inline-vs-heap split is an impl detail).
typedef struct SkaStringHashMapIterator {
    usize count;
    usize index;
    const char* key;
    void* value;
    bool _valid; // internal; true iff currently positioned on a live entry
} SkaStringHashMapIterator;

typedef struct SkaStringHashMapAddResult {
    void* value;
    usize hash;
    bool was_added;
} SkaStringHashMapAddResult;

// Generic hash map methods
SkaStringHashMap* ska_string_hash_map_init(usize capacity);
SkaStringHashMap* ska_string_hash_map_init_a(SkaAllocator allocator, usize capacity);
SkaStringHashMap* ska_string_hash_map_create_default_capacity();
bool ska_string_hash_map_deinit(SkaStringHashMap* hash_map);
SkaStringHashMapAddResult ska_string_hash_map_add(SkaStringHashMap* hash_map, const char* key, const void* value, usize value_size);
void* ska_string_hash_map_get(const SkaStringHashMap* hash_map, const char* key);
void* ska_string_hash_map_get_with_hash(const SkaStringHashMap* hash_map, const char* key, usize hash);
bool ska_string_hash_map_has(SkaStringHashMap* hash_map, const char* key);
bool ska_string_hash_map_set(SkaStringHashMap* hash_map, const char* key, const void* value, usize value_size);
bool ska_string_hash_map_erase(SkaStringHashMap* hash_map, const char* key);
// Int
SkaStringHashMapAddResult ska_string_hash_map_add_int(SkaStringHashMap* hash_map, const char* key, int value);
int ska_string_hash_map_get_int(SkaStringHashMap* hash_map, const char* key);
// String
SkaStringHashMapAddResult ska_string_hash_map_add_string(SkaStringHashMap* hash_map, const char* key, const char* value);
char* ska_string_hash_map_get_string(SkaStringHashMap* hash_map, const char* key);

// Iterator
SkaStringHashMapIterator ska_string_hash_map_iter_create(SkaStringHashMap* hash_map);
bool ska_string_hash_map_iter_is_valid(SkaStringHashMap* hash_map, SkaStringHashMapIterator* iterator);
void ska_string_hash_map_iter_advance(SkaStringHashMap* hash_map, SkaStringHashMapIterator* iterator);

#ifdef __cplusplus
}
#endif
