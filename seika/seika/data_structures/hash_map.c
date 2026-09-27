#include "hash_map.h"
#include <string.h>
#include "seika/assert.h"

static usize ska_default_hash(void* raw_key, usize key_size);
static int32 ska_default_compare(void* first_key, void* second_key, usize key_size);

static SkaHashMapNode* hash_map_create_node(SkaHashMap* hash_map, void* key, void* value, SkaHashMapNode* next);
static void hash_map_destroy_node(SkaHashMap* map, SkaHashMapNode* node);

static bool hash_map_push_front(SkaHashMap* hash_map, usize index, void* key, void* value);
static void hash_map_grow_if_needed(SkaHashMap* hash_map);
static void hash_map_shrink_if_needed(SkaHashMap* hash_map);
static void hash_map_allocate(SkaHashMap* hash_map, usize capacity);
static void hash_map_resize(SkaHashMap* hash_map, usize capacity);

SkaHashMap* ska_hash_map_init(usize key_size, usize value_size, usize capacity) {
    return ska_hash_map_init_a(ska_mem_get_global_allocator(), key_size, value_size, capacity);
}

SkaHashMap* ska_hash_map_init_a(SkaAllocator allocator, usize key_size, usize value_size, usize capacity) {
    SkaHashMap* map = SKA_ALLOC_A(allocator, SkaHashMap);
    map->allocator = allocator;
    map->key_size = key_size;
    map->value_size = value_size;
    map->size = 0;
    map->initial_capacity = capacity;
    map->hash_func = ska_default_hash;
    map->compare_func = ska_default_compare;
    hash_map_allocate(map, capacity);
    return map;
}

SkaHashMapNode* hash_map_create_node(SkaHashMap* hash_map, void* key, void* value, SkaHashMapNode* next) {
    SkaHashMapNode* node = (SkaHashMapNode*)SKA_ALLOC_A(hash_map->allocator, SkaHashMapNode);
    node->key = SKA_ALLOC_BYTES_A(hash_map->allocator, hash_map->key_size);
    node->value = SKA_ALLOC_BYTES_A(hash_map->allocator, hash_map->value_size);
    memcpy(node->key, key, hash_map->key_size);
    memcpy(node->value, value, hash_map->value_size);
    node->next = next;
    return node;
}

bool ska_hash_map_deinit(SkaHashMap* hash_map) {
    SkaHashMapNode* node = NULL;
    SkaHashMapNode* next = NULL;
    for (usize chain = 0; chain < hash_map->capacity; chain++) {
        node = hash_map->nodes[chain];
        while (node != NULL) {
            next = node->next;
            hash_map_destroy_node(hash_map, node);
            node = next;
        }
    }

    SKA_FREE_A(hash_map->allocator, hash_map->nodes);
    SKA_FREE_A(hash_map->allocator, hash_map);

    return true;
}

bool hash_map_push_front(SkaHashMap* hash_map, usize index, void* key, void* value) {
    hash_map->nodes[index] = hash_map_create_node(hash_map, key, value, hash_map->nodes[index]);
    return hash_map->nodes[index] != NULL;
}

SkaHashMapAddResult ska_hash_map_add(SkaHashMap* hash_map, const void* key, void* value) {
    SKA_ASSERT(hash_map != NULL);
    SKA_ASSERT(key != NULL);
    SKA_ASSERT(value != NULL);

    hash_map_grow_if_needed(hash_map);

    const usize index = hash_map->hash_func((void*)key, hash_map->key_size) % hash_map->capacity;
    void* existing_value = ska_hash_map_get(hash_map, key);
    if (existing_value) {
        static bool should_update = false;
        if (should_update) {
            memcpy(existing_value, value, hash_map->value_size);
        }
        return (SkaHashMapAddResult){
            .hash = index,
            .was_added =
                should_update}; // Updated Item (if shouldUpdate is true)
    }

    if (!hash_map_push_front(hash_map, index, (void*)key, value)) {
        // return false; // Error
        return (SkaHashMapAddResult){.hash = index,
                                     .was_added = false}; // Error
    }

    hash_map->size++;
    // Inserted
    return (SkaHashMapAddResult){.hash = index, .was_added = true};
}

bool ska_hash_map_has(const SkaHashMap* hash_map, const void* key) {
    SKA_ASSERT(hash_map != NULL);
    SKA_ASSERT(key != NULL);

    if (hash_map->size == 0) return false;

    const usize index = hash_map->hash_func((void*)key, hash_map->key_size) % hash_map->capacity;
    for (const SkaHashMapNode* node = hash_map->nodes[index]; node; node = node->next) {
        if (hash_map->compare_func((void*)key, node->key, hash_map->key_size) == 0) {
            return true;
        }
    }
    return false;
}

void* ska_hash_map_get(const SkaHashMap* hash_map, const void* key) {
    SKA_ASSERT(hash_map != NULL);
    const usize index = hash_map->hash_func((void*)key, hash_map->key_size) % hash_map->capacity;
    for (SkaHashMapNode* node = hash_map->nodes[index]; node; node = node->next) {
        if (hash_map->compare_func((void*)key, node->key, hash_map->key_size) == 0) {
            return node->value;
        }
    }
    return NULL;
}

void* ska_hash_map_get_with_hash(const SkaHashMap* hash_map, const void* key, usize hash) {
    SKA_ASSERT(hash_map != NULL);
    for (SkaHashMapNode* node = hash_map->nodes[hash]; node; node = node->next) {
        if (hash_map->compare_func((void*)key, node->key, hash_map->key_size) == 0) {
            return node->value;
        }
    }
    return NULL;
}


bool ska_hash_map_erase(SkaHashMap* hash_map, const void* key) {
    SKA_ASSERT(hash_map != NULL);
    SKA_ASSERT(key != NULL);

    const usize index = hash_map->hash_func((void*)key, hash_map->key_size) % hash_map->capacity;
    SkaHashMapNode* node = hash_map->nodes[index];
    for (SkaHashMapNode* previous = NULL; node; previous = node, node = node->next) {
        if (hash_map->compare_func((void*)key, node->key, hash_map->key_size) == 0) {
            if (previous != NULL) {
                previous->next = node->next;
            } else {
                hash_map->nodes[index] = node->next;
            }

            hash_map_destroy_node(hash_map, node);
            hash_map->size--;

            hash_map_shrink_if_needed(hash_map);

            return true;
        }
    }

    return false;
}

void ska_hash_map_clear(SkaHashMap* hash_map) {
    SKA_ASSERT(hash_map != NULL);

    // Free all nodes
    for (usize chain = 0; chain < hash_map->capacity; chain++) {
        SkaHashMapNode* node = hash_map->nodes[chain];
        while (node != NULL) {
            SkaHashMapNode* next = node->next;
            hash_map_destroy_node(hash_map, node);
            node = next;
        }
        hash_map->nodes[chain] = NULL;
    }

    // Reset size
    hash_map->size = 0;

    // Optionally shrink back to minimum capacity
    // if (hashMap->capacity > SKA_HASH_MAP_MIN_CAPACITY) {
    //     hash_map_resize(hashMap, SKA_HASH_MAP_MIN_CAPACITY);
    // }
}

void hash_map_grow_if_needed(SkaHashMap* hash_map) {
    if (hash_map->size >= hash_map->capacity) {
        hash_map_resize(hash_map, hash_map->size * 2);
    }
}

void hash_map_shrink_if_needed(SkaHashMap* hash_map) {
    const usize shrink_capacity = (usize)((float) hash_map->capacity * SKA_HASH_MAP_SHRINK_THRESHOLD);
    if (hash_map->size <= shrink_capacity && hash_map->size > hash_map->initial_capacity) {
        hash_map_resize(hash_map, shrink_capacity);
    }
}

void hash_map_allocate(SkaHashMap* hash_map, usize capacity) {
    // hashMap->nodes = (SkaHashMapNode**)SKA_ALLOC_BYTES(capacity * sizeof(SkaHashMapNode*));
    hash_map->nodes = (SkaHashMapNode**)SKA_ALLOC_BYTES_A(hash_map->allocator, capacity * sizeof(SkaHashMapNode*));
    memset(hash_map->nodes, 0, capacity * sizeof(SkaHashMapNode*));

    hash_map->capacity = capacity;
}

void hash_map_rehash(SkaHashMap* hash_map, SkaHashMapNode** old_node, usize old_capacity) {
    for (usize chain = 0; chain < old_capacity; chain++) {
        for (SkaHashMapNode* node = old_node[chain]; node != NULL;) {
            SkaHashMapNode* next = node->next;

            usize new_index = hash_map->hash_func(node->key, hash_map->key_size) % hash_map->capacity;
            node->next = hash_map->nodes[new_index];
            hash_map->nodes[new_index] = node;

            node = next;
        }
    }
}

void hash_map_resize(SkaHashMap* hash_map, usize capacity) {
    if (capacity < SKA_HASH_MAP_MIN_CAPACITY) {
        if (hash_map->capacity > SKA_HASH_MAP_MIN_CAPACITY) {
            capacity = SKA_HASH_MAP_MIN_CAPACITY;
        } else {
            // Do nothing since the passed in capacity is too low and the current map's capacity is already at min limit
            return;
        }
    }

    SkaHashMapNode** old_node = hash_map->nodes;
    usize old_capacity = hash_map->capacity;
    hash_map_allocate(hash_map, capacity);

    hash_map_rehash(hash_map, old_node, old_capacity);

    SKA_FREE_A(hash_map->allocator, old_node);
}

// --- Iterator --- //
SkaHashMapIterator ska_hash_map_iter_create(SkaHashMap* hash_map) {
    // Get initial node if exists
    SkaHashMapNode* initial_node = NULL;
    usize initial_index = 0;
    for (usize chain = 0; chain < hash_map->capacity; chain++) {
        SkaHashMapNode* node = hash_map->nodes[chain];
        if (node != NULL) {
            initial_node = node;
            initial_index = chain + 1;
            break;
        }
    }
    usize iterator_count = initial_node != NULL ? 1 : 0;
    SkaHashMapIterator iterator = { .count = iterator_count, .end = hash_map->capacity, .index = initial_index, .pair = initial_node };
    return iterator;
}

bool ska_hash_map_iter_is_valid(SkaHashMap* hash_map, SkaHashMapIterator* iterator) {
    return iterator->pair != NULL && iterator->count <= hash_map->size;
}

void ska_hash_map_iter_advance(SkaHashMap* hash_map, SkaHashMapIterator* iterator) {
    if (ska_hash_map_iter_is_valid(hash_map, iterator)) {
        if (iterator->pair->next != NULL) {
            iterator->pair = iterator->pair->next;
            iterator->count++;
            return;
        }

        // Search nodes array if there are no more linked pairs
        for (usize chain = iterator->index; chain < hash_map->capacity; chain++) {
            SkaHashMapNode* node = hash_map->nodes[chain];
            if (node != NULL) {
                iterator->pair = node;
                iterator->index = chain + 1;
                iterator->count++;
                return;
            }
        }
    }
    // Invalidate iterator since we've reached the end
    iterator->pair = NULL;
}

// Misc
usize ska_default_hash(void* raw_key, usize key_size) {
    // djb2 string hashing algorithm
    // sstp://www.cse.yorku.ca/~oz/hash.ssml
    usize byte;
    usize hash = 5381;
    char* key = (char*) raw_key;

    for (byte = 0; byte < key_size; ++byte) {
        // (hash << 5) + hash = hash * 33
        hash = ((hash << 5) + hash) ^ key[byte];
    }

    return hash;
}

int32 ska_default_compare(void* first_key, void* second_key, usize key_size) {
    return memcmp(first_key, second_key, key_size);
}

void hash_map_destroy_node(SkaHashMap* map, SkaHashMapNode* node) {
    SKA_ASSERT(node != NULL);
    SKA_ASSERT(node->value != NULL);
    SKA_ASSERT(node->key != NULL);

    SKA_FREE_A(map->allocator, node->key);
    node->key = NULL;
    SKA_FREE_A(map->allocator, node->value);
    node->value = NULL;
    SKA_FREE_A(map->allocator, node);
}
