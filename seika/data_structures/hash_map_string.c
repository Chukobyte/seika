#include "hash_map_string.h"
#include <string.h>
#include "seika/string.h"
#include "seika/assert.h"

// Grow trigger: (size + tombstones) / capacity >= 0.7. Tombstones count against load same as
// live entries -- they still occupy a slot in the probe sequence -- so a map churned via
// repeated add/erase cycles rehashes (and thereby reclaims its tombstones) even if `size`
// itself never grows. Matches koa/koa/stack_value_hash_map.c's constants for consistency
// across the codebase's two hand-written open-addressing hash maps.
#define SKA_STRING_MAP_GROW_LOAD_NUM 7
#define SKA_STRING_MAP_GROW_LOAD_DEN 10
// Shrink trigger (checked after erase only): size / capacity <= 0.25.
#define SKA_STRING_MAP_SHRINK_LOAD_NUM 1
#define SKA_STRING_MAP_SHRINK_LOAD_DEN 4

static usize ska_default_hash_string(const char* raw_key);
static int32 ska_default_compare_string(const char* first_key, const char* second_key);

static usize hash_map_next_pow2(usize n) {
    usize p = 1;
    while (p < n) p <<= 1;
    return p;
}

static const char* slot_key(const SkaStringHashMapSlot* slot) {
    return slot->key_heap ? slot->key_heap : slot->key_inline;
}

static void* slot_value(SkaStringHashMapSlot* slot) {
    return slot->value_heap ? slot->value_heap : slot->value_inline;
}

// Copies `key` into the slot (inline if it fits, one heap allocation otherwise). Frees any
// previously-heap-allocated key first (safe no-op if none was heap-allocated).
static void slot_set_key(SkaStringHashMap* map, SkaStringHashMapSlot* slot, const char* key) {
    if (slot->key_heap) {
        SKA_FREE_A(map->allocator, slot->key_heap);
        slot->key_heap = NULL;
    }
    usize len = strlen(key);
    slot->key_len = len;
    if (len < SKA_STRING_HASH_MAP_INLINE_KEY_CAP) {
        memcpy(slot->key_inline, key, len + 1);
    } else {
        slot->key_heap = (char*)SKA_ALLOC_BYTES_A(map->allocator, len + 1);
        memcpy(slot->key_heap, key, len + 1);
    }
}

// Copies `value` (value_size bytes) into the slot (inline if it fits, one heap allocation
// otherwise). Frees any previously-heap-allocated value first (safe no-op if none).
static void slot_set_value(SkaStringHashMap* map, SkaStringHashMapSlot* slot, const void* value, usize value_size) {
    if (slot->value_heap) {
        SKA_FREE_A(map->allocator, slot->value_heap);
        slot->value_heap = NULL;
    }
    slot->value_size = value_size;
    if (value_size <= SKA_STRING_HASH_MAP_INLINE_VALUE_CAP) {
        memcpy(slot->value_inline, value, value_size);
    } else {
        slot->value_heap = SKA_ALLOC_BYTES_A(map->allocator, value_size);
        memcpy(slot->value_heap, value, value_size);
    }
}

// Frees any heap-backed key/value payload for this slot (called before a slot is erased or
// the whole map is torn down). Does not touch .occupied/.tombstone/.hash -- callers manage
// those directly since the right end-state differs (erase vs. deinit).
static void slot_free_payload(SkaStringHashMap* map, SkaStringHashMapSlot* slot) {
    if (slot->key_heap) {
        SKA_FREE_A(map->allocator, slot->key_heap);
        slot->key_heap = NULL;
    }
    if (slot->value_heap) {
        SKA_FREE_A(map->allocator, slot->value_heap);
        slot->value_heap = NULL;
    }
}

static void hash_map_allocate(SkaStringHashMap* map, usize capacity) {
    map->slots = (SkaStringHashMapSlot*)SKA_ALLOC_BYTES_ZEROED_A(map->allocator, capacity * sizeof(SkaStringHashMapSlot));
    map->capacity = capacity;
}

// Re-inserts a known-live, known-unique-key slot into a freshly (zero-)allocated slots array --
// no equality check needed (the source map already guaranteed uniqueness), so this only has to
// find the first EMPTY slot in the new array's probe sequence. A plain struct-copy correctly
// carries the source slot's inline bytes AND any heap pointers along -- no re-copy needed.
static void hash_map_reinsert(SkaStringHashMap* map, const SkaStringHashMapSlot* src) {
    usize index = src->hash & (map->capacity - 1);
    while (map->slots[index].occupied) {
        index = (index + 1) & (map->capacity - 1);
    }
    map->slots[index] = *src;
}

static void hash_map_resize(SkaStringHashMap* map, usize new_capacity) {
    if (new_capacity < SKA_STRING_HASH_MAP_MIN_CAPACITY) new_capacity = SKA_STRING_HASH_MAP_MIN_CAPACITY;
    new_capacity = hash_map_next_pow2(new_capacity);
    if (new_capacity == map->capacity && map->tombstone_count == 0) return;

    SkaStringHashMapSlot* old_slots = map->slots;
    usize old_capacity = map->capacity;
    hash_map_allocate(map, new_capacity);
    for (usize i = 0; i < old_capacity; i++) {
        if (old_slots[i].occupied) {
            hash_map_reinsert(map, &old_slots[i]);
        }
    }
    SKA_FREE_A(map->allocator, old_slots);
    map->tombstone_count = 0;
}

static void hash_map_grow_if_needed(SkaStringHashMap* map) {
    const usize load = map->size + map->tombstone_count + 1;
    if (load * SKA_STRING_MAP_GROW_LOAD_DEN >= map->capacity * SKA_STRING_MAP_GROW_LOAD_NUM) {
        hash_map_resize(map, hash_map_next_pow2((map->size + 1) * 2));
    }
}

static void hash_map_shrink_if_needed(SkaStringHashMap* map) {
    const bool underfull = map->size * SKA_STRING_MAP_SHRINK_LOAD_DEN <= map->capacity * SKA_STRING_MAP_SHRINK_LOAD_NUM;
    if (underfull && map->capacity > map->initial_capacity) {
        usize target = hash_map_next_pow2(map->size * 2);
        if (target < map->initial_capacity) target = map->initial_capacity;
        if (target < map->capacity) hash_map_resize(map, target);
    }
}

// Returns the slot holding `key` (matched by cached hash then compare_func), or NULL if not
// present. Terminates on the first truly EMPTY slot in the probe sequence -- tombstones and
// non-matching occupied slots are skipped over, not treated as a stopping condition.
static SkaStringHashMapSlot* hash_map_find_slot(const SkaStringHashMap* map, const char* key, usize hash) {
    if (map->capacity == 0) return NULL;
    usize index = hash & (map->capacity - 1);
    for (;;) {
        SkaStringHashMapSlot* slot = &map->slots[index];
        if (!slot->occupied && !slot->tombstone) return NULL;
        if (slot->occupied && slot->hash == hash && map->compare_func(key, slot_key(slot)) == 0) return slot;
        index = (index + 1) & (map->capacity - 1);
    }
}

SkaStringHashMap* ska_string_hash_map_init(usize capacity) {
    return ska_string_hash_map_init_a(ska_mem_get_global_allocator(), capacity);
}

SkaStringHashMap* ska_string_hash_map_init_a(SkaAllocator allocator, usize capacity) {
    SkaStringHashMap* map = SKA_ALLOC_A(allocator, SkaStringHashMap);
    map->allocator = allocator;
    map->size = 0;
    map->tombstone_count = 0;
    map->hash_func = ska_default_hash_string;
    map->compare_func = ska_default_compare_string;
    capacity = hash_map_next_pow2(capacity < SKA_STRING_HASH_MAP_MIN_CAPACITY ? SKA_STRING_HASH_MAP_MIN_CAPACITY : capacity);
    map->initial_capacity = capacity;
    map->slots = NULL;
    hash_map_allocate(map, capacity);
    return map;
}

SkaStringHashMap* ska_string_hash_map_create_default_capacity() {
    return ska_string_hash_map_init(SKA_STRING_HASH_MAP_MIN_CAPACITY);
}

bool ska_string_hash_map_deinit(SkaStringHashMap* hash_map) {
    for (usize i = 0; i < hash_map->capacity; i++) {
        if (hash_map->slots[i].occupied) {
            slot_free_payload(hash_map, &hash_map->slots[i]);
        }
    }
    SKA_FREE_A(hash_map->allocator, hash_map->slots);
    SKA_FREE_A(hash_map->allocator, hash_map);
    return true;
}

SkaStringHashMapAddResult ska_string_hash_map_add(SkaStringHashMap* hash_map, const char* key, const void* value, usize value_size) {
    SKA_ASSERT(hash_map != NULL);
    SKA_ASSERT(key != NULL);
    SKA_ASSERT(value != NULL);

    hash_map_grow_if_needed(hash_map);
    const usize hash = hash_map->hash_func(key);

    usize index = hash & (hash_map->capacity - 1);
    usize first_tombstone = SIZE_MAX;
    for (;;) {
        SkaStringHashMapSlot* slot = &hash_map->slots[index];
        if (slot->tombstone) {
            if (first_tombstone == SIZE_MAX) first_tombstone = index;
        } else if (slot->occupied) {
            if (slot->hash == hash && hash_map->compare_func(key, slot_key(slot)) == 0) {
                // Matches this hash map's original behavior: a duplicate `add` does NOT
                // update the existing value (should_update_value was always hardcoded
                // false in the prior separate-chaining implementation).
                return (SkaStringHashMapAddResult){ .value = slot_value(slot), .hash = index, .was_added = false };
            }
        } else {
            // Truly empty -- key is not present. Insert at the first tombstone seen along
            // the way (shortens future probe sequences) or here if none was seen.
            const usize insert_at = (first_tombstone != SIZE_MAX) ? first_tombstone : index;
            SkaStringHashMapSlot* dest = &hash_map->slots[insert_at];
            if (dest->tombstone) hash_map->tombstone_count--;
            dest->occupied = true;
            dest->tombstone = false;
            dest->hash = hash;
            slot_set_key(hash_map, dest, key);
            slot_set_value(hash_map, dest, value, value_size);
            hash_map->size++;
            return (SkaStringHashMapAddResult){ .value = slot_value(dest), .hash = insert_at, .was_added = true };
        }
        index = (index + 1) & (hash_map->capacity - 1);
    }
}

bool ska_string_hash_map_has(SkaStringHashMap* hash_map, const char* key) {
    SKA_ASSERT(hash_map != NULL);
    return hash_map_find_slot(hash_map, key, hash_map->hash_func(key)) != NULL;
}

void* ska_string_hash_map_get(const SkaStringHashMap* hash_map, const char* key) {
    SKA_ASSERT_FMT(hash_map != NULL, "Trying to get key '%s' from a NULL hashmap!", key);
    SkaStringHashMapSlot* slot = hash_map_find_slot(hash_map, key, hash_map->hash_func(key));
    return slot ? slot_value(slot) : NULL;
}

// NOTE: the prior separate-chaining implementation treated `hash` as a raw bucket-array
// index (`nodes[hash]`, no masking, no probing) -- open addressing has no equivalent of
// "here's the bucket, walk its chain," so this treats `hash` as a genuine hash value and
// probes from there instead. Zero callers exist anywhere in this codebase (confirmed by
// search); kept only for API compatibility.
void* ska_string_hash_map_get_with_hash(const SkaStringHashMap* hash_map, const char* key, usize hash) {
    SKA_ASSERT_FMT(hash_map != NULL, "Trying to get key '%s' from a NULL hashmap!", key);
    SkaStringHashMapSlot* slot = hash_map_find_slot(hash_map, key, hash);
    return slot ? slot_value(slot) : NULL;
}

bool ska_string_hash_map_set(SkaStringHashMap* hash_map, const char* key, const void* value, usize value_size) {
    SKA_ASSERT(hash_map != NULL);
    SKA_ASSERT(key != NULL);
    SKA_ASSERT(value != NULL);

    SkaStringHashMapSlot* slot = hash_map_find_slot(hash_map, key, hash_map->hash_func(key));
    if (slot) {
        slot_set_value(hash_map, slot, value, value_size);
        return true;
    }
    return ska_string_hash_map_add(hash_map, key, value, value_size).was_added;
}

bool ska_string_hash_map_erase(SkaStringHashMap* hash_map, const char* key) {
    SKA_ASSERT(hash_map != NULL);
    SKA_ASSERT(key != NULL);

    if (hash_map->size == 0) return false;
    SkaStringHashMapSlot* slot = hash_map_find_slot(hash_map, key, hash_map->hash_func(key));
    if (!slot) return false;

    slot_free_payload(hash_map, slot);
    slot->occupied = false;
    slot->tombstone = true;
    slot->key_len = 0;
    slot->value_size = 0;
    hash_map->size--;
    hash_map->tombstone_count++;

    hash_map_shrink_if_needed(hash_map);

    return true;
}

// Int
SkaStringHashMapAddResult ska_string_hash_map_add_int(SkaStringHashMap* hash_map, const char* key, int32 value) {
    return ska_string_hash_map_add(hash_map, key, &value, sizeof(int32));
}

int32 ska_string_hash_map_get_int(SkaStringHashMap* hash_map, const char* key) {
    return *(int32*)ska_string_hash_map_get(hash_map, key);
}

// String
SkaStringHashMapAddResult ska_string_hash_map_add_string(SkaStringHashMap* hash_map, const char* key, const char* value) {
    // The characters themselves (with terminator) are the value; get_string returns them in place
    return ska_string_hash_map_add(hash_map, key, value, strlen(value) + 1);
}

char* ska_string_hash_map_get_string(SkaStringHashMap* hash_map, const char* key) {
    return (char*)ska_string_hash_map_get(hash_map, key);
}

//--- Iterator ---//
SkaStringHashMapIterator ska_string_hash_map_iter_create(SkaStringHashMap* hash_map) {
    SkaStringHashMapIterator it = { .count = 0, .index = 0, .key = NULL, .value = NULL, ._valid = false };
    for (usize i = 0; i < hash_map->capacity; i++) {
        if (hash_map->slots[i].occupied) {
            it.key = slot_key(&hash_map->slots[i]);
            it.value = slot_value(&hash_map->slots[i]);
            it.index = i + 1;
            it.count = 1;
            it._valid = true;
            return it;
        }
    }
    return it;
}

bool ska_string_hash_map_iter_is_valid(SkaStringHashMap* hash_map, SkaStringHashMapIterator* iterator) {
    return iterator->_valid && iterator->count <= hash_map->size;
}

void ska_string_hash_map_iter_advance(SkaStringHashMap* hash_map, SkaStringHashMapIterator* iterator) {
    if (ska_string_hash_map_iter_is_valid(hash_map, iterator)) {
        for (usize i = iterator->index; i < hash_map->capacity; i++) {
            if (hash_map->slots[i].occupied) {
                iterator->key = slot_key(&hash_map->slots[i]);
                iterator->value = slot_value(&hash_map->slots[i]);
                iterator->index = i + 1;
                iterator->count++;
                return;
            }
        }
    }
    // Invalidate iterator since we've reached the end
    iterator->key = NULL;
    iterator->value = NULL;
    iterator->_valid = false;
}

// Misc
usize ska_default_hash_string(const char* raw_key) {
    // djb2 string hashing algorithm
    // sstp://www.cse.yorku.ca/~oz/hash.ssml
    usize byte;
    usize hash = 5381;
    usize key_size = strlen(raw_key);

    for (byte = 0; byte < key_size; ++byte) {
        hash = ((hash << 5) + hash) ^ raw_key[byte];
    }

    return hash;
}

int32 ska_default_compare_string(const char* first_key, const char* second_key) {
    return strcmp(first_key, second_key);
}
