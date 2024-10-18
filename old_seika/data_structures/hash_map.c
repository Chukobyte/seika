#include "hash_map.h"

#include <string.h>

#include "seika/memory.h"
#include "seika/assert.h"

static usize se_default_hash(void* raw_key, usize key_size);
static int32 se_default_compare(void* first_key, void* second_key, usize key_size);

static SkaHashMapNode* hash_map_create_node(SkaHashMap* hashMap, void* key, void* value, SkaHashMapNode* next);
static void hash_map_destroy_node(SkaHashMapNode* node);

static bool hash_map_push_front(SkaHashMap* hashMap, usize index, void* key, void* value);
static void hash_map_grow_if_needed(SkaHashMap* hashMap);
static void hash_map_shrink_if_needed(SkaHashMap* hashMap);
static void hash_map_allocate(SkaHashMap* hashMap, usize capacity);
static void hash_map_resize(SkaHashMap* hashMap, usize capacity);

SkaHashMap* ska_hash_map_create(usize keySize, usize valueSize, usize capacity) {
    SkaHashMap* map = (SkaHashMap*) SKA_MEM_ALLOCATE_SIZE(sizeof(SkaHashMap));
    map->keySize = keySize;
    map->valueSize = valueSize;
    map->size = 0;
    map->hashFunc = se_default_hash;
    map->compareFunc = se_default_compare;
    hash_map_allocate(map, capacity);
    return map;
}

SkaHashMapNode* hash_map_create_node(SkaHashMap* hashMap, void* key, void* value, SkaHashMapNode* next) {
    SkaHashMapNode* node = (SkaHashMapNode*) SKA_MEM_ALLOCATE(SkaHashMapNode);
    node->key = SKA_MEM_ALLOCATE_SIZE(hashMap->keySize);
    node->value = SKA_MEM_ALLOCATE_SIZE(hashMap->valueSize);
    memcpy(node->key, key, hashMap->keySize);
    memcpy(node->value, value, hashMap->valueSize);
    node->next = next;
    return node;
}

bool ska_hash_map_destroy(SkaHashMap* hashMap) {
    SkaHashMapNode* node = NULL;
    SkaHashMapNode* next = NULL;
    for (usize chain = 0; chain < hashMap->capacity; chain++) {
        node = hashMap->nodes[chain];
        while (node != NULL) {
            next = node->next;
            hash_map_destroy_node(node);
            node = next;
        }
    }

    SKA_MEM_FREE(hashMap);

    return true;
}

bool hash_map_push_front(SkaHashMap* hashMap, usize index, void* key, void* value) {
    hashMap->nodes[index] = hash_map_create_node(hashMap, key, value, hashMap->nodes[index]);
    return hashMap->nodes[index] != NULL;
}

bool ska_hash_map_add(SkaHashMap* hashMap, void* key, void* value) {
    SKA_ASSERT(hashMap != NULL);
    SKA_ASSERT(key != NULL);
    SKA_ASSERT(value != NULL);

    hash_map_grow_if_needed(hashMap);

    usize index = hashMap->hashFunc(key, hashMap->keySize) % hashMap->capacity;
    if (ska_hash_map_has(hashMap, key)) {
        SkaHashMapNode* node = hashMap->nodes[index];
        memcpy(node->value, value, hashMap->valueSize);
        return true; // Updated Item
    }

    if (!hash_map_push_front(hashMap, index, key, value)) {
        return false; // Error
    }

    hashMap->size++;
    // Inserted
    return true;
}

bool ska_hash_map_has(SkaHashMap* hashMap, void* key) {
    SKA_ASSERT(hashMap != NULL);
    usize index = hashMap->hashFunc(key, hashMap->keySize) % hashMap->capacity;
    for (SkaHashMapNode* node = hashMap->nodes[index]; node; node = node->next) {
        if (hashMap->compareFunc(key, node->key, hashMap->keySize) == 0) {
            return true;
        }
    }
    return false;
}

void* ska_hash_map_get(SkaHashMap* hashMap, void* key) {
    SKA_ASSERT(hashMap != NULL);
    usize index = hashMap->hashFunc(key, hashMap->keySize) % hashMap->capacity;
    for (SkaHashMapNode* node = hashMap->nodes[index]; node; node = node->next) {
        if (hashMap->compareFunc(key, node->key, hashMap->keySize) == 0) {
            return node->value;
        }
    }
    return NULL;
}

bool ska_hash_map_erase(SkaHashMap* hashMap, void* key) {
    SKA_ASSERT(hashMap != NULL);
    SKA_ASSERT(key != NULL);

    usize index = hashMap->hashFunc(key, hashMap->keySize) % hashMap->capacity;
    SkaHashMapNode* node = hashMap->nodes[index];
    for (SkaHashMapNode* previous = NULL; node; previous = node, node = node->next) {
        if (hashMap->compareFunc(key, node->key, hashMap->keySize) == 0) {
            if (previous != NULL) {
                previous->next = node->next;
            } else {
                hashMap->nodes[index] = node->next;
            }

            hash_map_destroy_node(node);
            hashMap->size--;

            hash_map_shrink_if_needed(hashMap);

            return true;
        }
    }

    return false;
}

void hash_map_grow_if_needed(SkaHashMap* hashMap) {
    SKA_ASSERT_FMT(hashMap->size <= hashMap->capacity, "Hashmap size '%d' is bigger than its capacity '%d'!", hashMap->size, hashMap->capacity);
    if (hashMap->size == hashMap->capacity) {
        hash_map_resize(hashMap, hashMap->size * 2);
    }
}

void hash_map_shrink_if_needed(SkaHashMap* hashMap) {
    SKA_ASSERT_FMT(hashMap->size <= hashMap->capacity, "Hashmap size '%d' is bigger than its capacity '%d'!", hashMap->size, hashMap->capacity);
    usize shrinkCapacity = (usize) ((float) hashMap->capacity * SKA_HASH_MAP_SHRINK_THRESHOLD);
    if (hashMap->size == shrinkCapacity) {
        hash_map_resize(hashMap, shrinkCapacity);
    }
}

void hash_map_allocate(SkaHashMap* hashMap, usize capacity) {
    hashMap->nodes = (SkaHashMapNode**)SKA_MEM_ALLOCATE_SIZE(capacity * sizeof(SkaHashMapNode*));
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

    SKA_MEM_FREE(oldNode);
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
usize se_default_hash(void* raw_key, usize key_size) {
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

int32 se_default_compare(void* first_key, void* second_key, usize key_size) {
    return memcmp(first_key, second_key, key_size);
}

void hash_map_destroy_node(SkaHashMapNode* node) {
    SKA_ASSERT(node != NULL);
    SKA_ASSERT(node->value != NULL);
    SKA_ASSERT(node->key != NULL);

    SKA_MEM_FREE(node->key);
    node->key = NULL;
    SKA_MEM_FREE(node->value);
    node->value = NULL;
    SKA_MEM_FREE(node);
}
