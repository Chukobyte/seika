#include "hash_map.h"
#include <string.h>
#include "seika/assert.h"

static usize ska_default_hash(void* raw_key, usize key_size);
static int32 ska_default_compare(void* first_key, void* second_key, usize key_size);

static SkaHashMapNode* hash_map_create_node(SkaHashMap* hashMap, void* key, void* value, SkaHashMapNode* next);
static void hash_map_destroy_node(SkaHashMap* map, SkaHashMapNode* node);

static bool hash_map_push_front(SkaHashMap* hashMap, usize index, void* key, void* value);
static void hash_map_grow_if_needed(SkaHashMap* hashMap);
static void hash_map_shrink_if_needed(SkaHashMap* hashMap);
static void hash_map_allocate(SkaHashMap* hashMap, usize capacity);
static void hash_map_resize(SkaHashMap* hashMap, usize capacity);

SkaHashMap* ska_hash_map_init(usize keySize, usize valueSize, usize capacity) {
    return ska_hash_map_init_a(ska_mem_get_global_allocator(), keySize, valueSize, capacity);
}

SkaHashMap* ska_hash_map_init_a(SkaAllocator allocator, usize keySize, usize valueSize, usize capacity) {
    SkaHashMap* map = SKA_ALLOC_A(allocator, SkaHashMap);
    map->allocator = allocator;
    map->keySize = keySize;
    map->valueSize = valueSize;
    map->size = 0;
    map->initialCapacity = capacity;
    map->hashFunc = ska_default_hash;
    map->compareFunc = ska_default_compare;
    hash_map_allocate(map, capacity);
    return map;
}

SkaHashMapNode* hash_map_create_node(SkaHashMap* hashMap, void* key, void* value, SkaHashMapNode* next) {
    SkaHashMapNode* node = (SkaHashMapNode*)SKA_ALLOC_A(hashMap->allocator, SkaHashMapNode);
    node->key = SKA_ALLOC_BYTES_A(hashMap->allocator, hashMap->keySize);
    node->value = SKA_ALLOC_BYTES_A(hashMap->allocator, hashMap->valueSize);
    memcpy(node->key, key, hashMap->keySize);
    memcpy(node->value, value, hashMap->valueSize);
    node->next = next;
    return node;
}

bool ska_hash_map_deinit(SkaHashMap* hashMap) {
    SkaHashMapNode* node = NULL;
    SkaHashMapNode* next = NULL;
    for (usize chain = 0; chain < hashMap->capacity; chain++) {
        node = hashMap->nodes[chain];
        while (node != NULL) {
            next = node->next;
            hash_map_destroy_node(hashMap, node);
            node = next;
        }
    }

    SKA_FREE_A(hashMap->allocator, hashMap);

    return true;
}

bool hash_map_push_front(SkaHashMap* hashMap, usize index, void* key, void* value) {
    hashMap->nodes[index] = hash_map_create_node(hashMap, key, value, hashMap->nodes[index]);
    return hashMap->nodes[index] != NULL;
}

SkaHashMapAddResult ska_hash_map_add(SkaHashMap* hashMap, void* key, void* value) {
    SKA_ASSERT(hashMap != NULL);
    SKA_ASSERT(key != NULL);
    SKA_ASSERT(value != NULL);

    hash_map_grow_if_needed(hashMap);

    const usize index = hashMap->hashFunc(key, hashMap->keySize) % hashMap->capacity;
    void* existingValue = ska_hash_map_get(hashMap, key);
    if (existingValue) {
        static bool shouldUpdate = false;
        if (shouldUpdate) {
            memcpy(existingValue, value, hashMap->valueSize);
        }
        return (SkaHashMapAddResult){ .hash = index, .wasAdded = shouldUpdate }; // Updated Item (if shouldUpdate is true)
    }

    if (!hash_map_push_front(hashMap, index, key, value)) {
        // return false; // Error
        return (SkaHashMapAddResult){ .hash = index, .wasAdded = false }; // Error
    }

    hashMap->size++;
    // Inserted
    return (SkaHashMapAddResult){ .hash = index, .wasAdded = true };
}

bool ska_hash_map_has(const SkaHashMap* hashMap, void* key) {
    SKA_ASSERT(hashMap != NULL);
    SKA_ASSERT(key != NULL);

    if (hashMap->size == 0) return false;

    const usize index = hashMap->hashFunc(key, hashMap->keySize) % hashMap->capacity;
    for (const SkaHashMapNode* node = hashMap->nodes[index]; node; node = node->next) {
        if (hashMap->compareFunc(key, node->key, hashMap->keySize) == 0) {
            return true;
        }
    }
    return false;
}

void* ska_hash_map_get(const SkaHashMap* hashMap, void* key) {
    SKA_ASSERT(hashMap != NULL);
    const usize index = hashMap->hashFunc(key, hashMap->keySize) % hashMap->capacity;
    for (SkaHashMapNode* node = hashMap->nodes[index]; node; node = node->next) {
        if (hashMap->compareFunc(key, node->key, hashMap->keySize) == 0) {
            return node->value;
        }
    }
    return NULL;
}

void* ska_hash_map_get_with_hash(const SkaHashMap* hashMap, void* key, usize hash) {
    SKA_ASSERT(hashMap != NULL);
    for (SkaHashMapNode* node = hashMap->nodes[hash]; node; node = node->next) {
        if (hashMap->compareFunc(key, node->key, hashMap->keySize) == 0) {
            return node->value;
        }
    }
    return NULL;
}


bool ska_hash_map_erase(SkaHashMap* hashMap, void* key) {
    SKA_ASSERT(hashMap != NULL);
    SKA_ASSERT(key != NULL);

    const usize index = hashMap->hashFunc(key, hashMap->keySize) % hashMap->capacity;
    SkaHashMapNode* node = hashMap->nodes[index];
    for (SkaHashMapNode* previous = NULL; node; previous = node, node = node->next) {
        if (hashMap->compareFunc(key, node->key, hashMap->keySize) == 0) {
            if (previous != NULL) {
                previous->next = node->next;
            } else {
                hashMap->nodes[index] = node->next;
            }

            hash_map_destroy_node(hashMap, node);
            hashMap->size--;

            hash_map_shrink_if_needed(hashMap);

            return true;
        }
    }

    return false;
}

void ska_hash_map_clear(SkaHashMap* hashMap) {
    SKA_ASSERT(hashMap != NULL);

    // Free all nodes
    for (usize chain = 0; chain < hashMap->capacity; chain++) {
        SkaHashMapNode* node = hashMap->nodes[chain];
        while (node != NULL) {
            SkaHashMapNode* next = node->next;
            hash_map_destroy_node(hashMap, node);
            node = next;
        }
        hashMap->nodes[chain] = NULL;
    }

    // Reset size
    hashMap->size = 0;

    // Optionally shrink back to minimum capacity
    // if (hashMap->capacity > SKA_HASH_MAP_MIN_CAPACITY) {
    //     hash_map_resize(hashMap, SKA_HASH_MAP_MIN_CAPACITY);
    // }
}

void hash_map_grow_if_needed(SkaHashMap* hashMap) {
    if (hashMap->size >= hashMap->capacity) {
        hash_map_resize(hashMap, hashMap->size * 2);
    }
}

void hash_map_shrink_if_needed(SkaHashMap* hashMap) {
    const usize shrinkCapacity = (usize)((float) hashMap->capacity * SKA_HASH_MAP_SHRINK_THRESHOLD);
    if (hashMap->size <= shrinkCapacity && hashMap->size > hashMap->initialCapacity) {
        hash_map_resize(hashMap, shrinkCapacity);
    }
}

void hash_map_allocate(SkaHashMap* hashMap, usize capacity) {
    // hashMap->nodes = (SkaHashMapNode**)SKA_ALLOC_BYTES(capacity * sizeof(SkaHashMapNode*));
    hashMap->nodes = (SkaHashMapNode**)SKA_ALLOC_BYTES_A(hashMap->allocator, capacity * sizeof(SkaHashMapNode*));
    memset(hashMap->nodes, 0, capacity * sizeof(SkaHashMapNode*));

    hashMap->capacity = capacity;
}

void hash_map_rehash(SkaHashMap* hashMap, SkaHashMapNode** oldNode, usize oldCapacity) {
    for (usize chain = 0; chain < oldCapacity; chain++) {
        for (SkaHashMapNode* node = oldNode[chain]; node != NULL;) {
            SkaHashMapNode* next = node->next;

            usize newIndex = hashMap->hashFunc(node->key, hashMap->keySize) % hashMap->capacity;
            node->next = hashMap->nodes[newIndex];
            hashMap->nodes[newIndex] = node;

            node = next;
        }
    }
}

void hash_map_resize(SkaHashMap* hashMap, usize capacity) {
    if (capacity < SKA_HASH_MAP_MIN_CAPACITY) {
        if (hashMap->capacity > SKA_HASH_MAP_MIN_CAPACITY) {
            capacity = SKA_HASH_MAP_MIN_CAPACITY;
        } else {
            // Do nothing since the passed in capacity is too low and the current map's capacity is already at min limit
            return;
        }
    }

    SkaHashMapNode** oldNode = hashMap->nodes;
    usize oldCapacity = hashMap->capacity;
    hash_map_allocate(hashMap, capacity);

    hash_map_rehash(hashMap, oldNode, oldCapacity);

    SKA_FREE_A(hashMap->allocator, oldNode);
}

// --- Iterator --- //
SkaHashMapIterator ska_hash_map_iter_create(SkaHashMap* hashMap) {
    // Get initial node if exists
    SkaHashMapNode* initialNode = NULL;
    usize initialIndex = 0;
    for (usize chain = 0; chain < hashMap->capacity; chain++) {
        SkaHashMapNode* node = hashMap->nodes[chain];
        if (node != NULL) {
            initialNode = node;
            initialIndex = chain + 1;
            break;
        }
    }
    usize iteratorCount = initialNode != NULL ? 1 : 0;
    SkaHashMapIterator iterator = { .count = iteratorCount, .end = hashMap->capacity, .index = initialIndex, .pair = initialNode };
    return iterator;
}

bool ska_hash_map_iter_is_valid(SkaHashMap* hashMap, SkaHashMapIterator* iterator) {
    return iterator->pair != NULL && iterator->count <= hashMap->size;
}

void ska_hash_map_iter_advance(SkaHashMap* hashMap, SkaHashMapIterator* iterator) {
    if (ska_hash_map_iter_is_valid(hashMap, iterator)) {
        if (iterator->pair->next != NULL) {
            iterator->pair = iterator->pair->next;
            iterator->count++;
            return;
        }

        // Search nodes array if there are no more linked pairs
        for (usize chain = iterator->index; chain < hashMap->capacity; chain++) {
            SkaHashMapNode* node = hashMap->nodes[chain];
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
