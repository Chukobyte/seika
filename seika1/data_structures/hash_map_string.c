#include "hash_map_string.h"

#include <string.h>

#include "seika/string.h"
#include "seika/memory.h"
#include "seika/assert.h"

static usize ska_default_hash_string(const char* raw_key);
static int32 ska_default_compare_string(const char* first_key, const char* second_key);

static SkaStringHashMapNode* hash_map_create_node_string(SkaStringHashMap* hashMap, const char* key, const void* value, usize valueSize, SkaStringHashMapNode* next);
static void hash_map_destroy_node_string(SkaStringHashMapNode* node);

static bool hash_map_push_front_string(SkaStringHashMap* hashMap, usize index, const char* key, const void* value, usize valueSize);
static void string_hash_map_grow_if_needed(SkaStringHashMap* hashMap);
static void string_hash_map_shrink_if_needed(SkaStringHashMap* hashMap);
static void string_hash_map_allocate(SkaStringHashMap* hashMap, usize capacity);
static void string_hash_map_rehash(SkaStringHashMap* hashMap, SkaStringHashMapNode** oldNode, usize oldCapacity);
static void string_hash_map_resize(SkaStringHashMap* hashMap, usize newCapacity);

SkaStringHashMap* ska_string_hash_map_create(usize capacity) {
    SkaStringHashMap* map = (SkaStringHashMap*)SKA_MEM_ALLOCATE(SkaStringHashMap);
    map->capacity = capacity;
    map->size = 0;
    map->hashFunc = ska_default_hash_string;
    map->compareFunc = ska_default_compare_string;
    map->nodes = (SkaStringHashMapNode**) SKA_MEM_ALLOCATE_SIZE(capacity * sizeof(SkaStringHashMapNode*));
    memset(map->nodes, 0, capacity * sizeof(SkaStringHashMapNode*)); // TODO: fix
    return map;
}

SkaStringHashMap* ska_string_hash_map_create_default_capacity() {
    return ska_string_hash_map_create(SKA_STRING_HASH_MAP_MIN_CAPACITY);
}

SkaStringHashMapNode* hash_map_create_node_string(SkaStringHashMap* hashMap, const char* key, const void* value, usize valueSize, SkaStringHashMapNode* next) {
    SkaStringHashMapNode* node = (SkaStringHashMapNode*) SKA_MEM_ALLOCATE_SIZE(sizeof(SkaStringHashMapNode));
    node->key = ska_strdup(key);
    node->value = SKA_MEM_ALLOCATE_SIZE(valueSize);
    memcpy(node->value, value, valueSize);
    node->valueSize = valueSize;
    node->next = next;
    return node;
}

bool ska_string_hash_map_destroy(SkaStringHashMap* hashMap) {
    SkaStringHashMapNode* node = NULL;
    SkaStringHashMapNode* next = NULL;
    for (usize chain = 0; chain < hashMap->capacity; chain++) {
        node = hashMap->nodes[chain];
        while (node) {
            next = node->next;
            hash_map_destroy_node_string(node);
            node = next;
        }
    }

    SKA_MEM_FREE(hashMap);

    return true;
}

bool hash_map_push_front_string(SkaStringHashMap* hashMap, usize index, const char* key, const void* value, usize valueSize) {
    hashMap->nodes[index] = hash_map_create_node_string(hashMap, key, value, valueSize, hashMap->nodes[index]);
    return hashMap->nodes[index] != NULL;
}

bool ska_string_hash_map_add(SkaStringHashMap* hashMap, const char* key, const void* value, usize valueSize) {
    SKA_ASSERT(hashMap != NULL);
    SKA_ASSERT(key != NULL);
    SKA_ASSERT(value != NULL);

    string_hash_map_grow_if_needed(hashMap);

    usize index = hashMap->hashFunc(key) % hashMap->capacity;
    if (ska_string_hash_map_has(hashMap, key)) {
        SkaStringHashMapNode* node = hashMap->nodes[index];
        memcpy(node->value, value, valueSize);
        return true; // Updated Item
    }

    if (!hash_map_push_front_string(hashMap, index, key, value, valueSize)) {
        return false; // Error
    }

    hashMap->size++;
    // Inserted
    return true;
}

bool ska_string_hash_map_has(SkaStringHashMap* hashMap, const char* key) {
    SKA_ASSERT(hashMap != NULL);
    usize index = hashMap->hashFunc(key) % hashMap->capacity;
    for (SkaStringHashMapNode* node = hashMap->nodes[index]; node; node = node->next) {
        if (hashMap->compareFunc(key, node->key) == 0) {
            return true;
        }
    }
    return false;
}

void* ska_string_hash_map_get(SkaStringHashMap* hashMap, const char* key) {
    SKA_ASSERT_FMT(hashMap != NULL, "Trying to get key '%s' from a NULL hashmap!", key);
    usize index = hashMap->hashFunc(key) % hashMap->capacity;
    for (SkaStringHashMapNode* node = hashMap->nodes[index]; node; node = node->next) {
        if (hashMap->compareFunc(key, node->key) == 0) {
            return node->value;
        }
    }
    return NULL;
}

// TODO: Make better...
void* ska_string_hash_map_find(SkaStringHashMap* hashMap, const char* key) {
    if (ska_string_hash_map_has(hashMap, key)) {
        return ska_string_hash_map_get(hashMap, key);
    }
    return NULL;
}

bool ska_string_hash_map_erase(SkaStringHashMap* hashMap, const char* key) {
    SKA_ASSERT(hashMap != NULL);
    SKA_ASSERT(key != NULL);

    usize index = hashMap->hashFunc(key) % hashMap->capacity;
    SkaStringHashMapNode* node = hashMap->nodes[index];
    for (SkaStringHashMapNode* previous = NULL; node; previous = node, node = node->next) {
        if (hashMap->compareFunc(key, node->key) == 0) {
            if (previous != NULL) {
                previous->next = node->next;
            } else {
                hashMap->nodes[index] = node->next;
            }

            hash_map_destroy_node_string(node);
            hashMap->size--;

            string_hash_map_shrink_if_needed(hashMap);

            return true;
        }
    }

    return false;
}

void string_hash_map_grow_if_needed(SkaStringHashMap* hashMap) {
    SKA_ASSERT_FMT(hashMap->size <= hashMap->capacity, "Hashmap size '%d' is bigger than its capacity '%d'!", hashMap->size, hashMap->capacity);
    if (hashMap->size == hashMap->capacity) {
        string_hash_map_resize(hashMap, hashMap->size * 2);
    }
}

void string_hash_map_shrink_if_needed(SkaStringHashMap* hashMap) {
    SKA_ASSERT_FMT(hashMap->size <= hashMap->capacity, "Hashmap size '%d' is bigger than its capacity '%d'!", hashMap->size, hashMap->capacity);
    usize shrinkCapacity = (usize) ((float) hashMap->capacity * SKA_STRING_HASH_MAP_SHRINK_THRESHOLD);
    if (hashMap->size == shrinkCapacity) {
        string_hash_map_resize(hashMap, shrinkCapacity);
    }
}

void string_hash_map_allocate(SkaStringHashMap* hashMap, usize capacity) {
    hashMap->nodes = SKA_MEM_ALLOCATE_SIZE(capacity * sizeof(SkaStringHashMapNode*));
    memset(hashMap->nodes, 0, capacity * sizeof(SkaStringHashMapNode*));

    hashMap->capacity = capacity;
}

void string_hash_map_rehash(SkaStringHashMap* hashMap, SkaStringHashMapNode** oldNode, usize oldCapacity) {
    for (usize chain = 0; chain < oldCapacity; chain++) {
        for (SkaStringHashMapNode* node = oldNode[chain]; node != NULL;) {
            SkaStringHashMapNode* next = node->next;

            usize newIndex = hashMap->hashFunc(node->key) % hashMap->capacity;
            node->next = hashMap->nodes[newIndex];
            hashMap->nodes[newIndex] = node;

            node = next;
        }
    }
}

void string_hash_map_resize(SkaStringHashMap* hashMap, usize newCapacity) {
    if (newCapacity < SKA_STRING_HASH_MAP_MIN_CAPACITY) {
        if (hashMap->capacity > SKA_STRING_HASH_MAP_MIN_CAPACITY) {
            newCapacity = SKA_STRING_HASH_MAP_MIN_CAPACITY;
        } else {
            // Do nothing since the passed in capacity is too low and the current map's capacity is already at min limit
            return;
        }
    }

    SkaStringHashMapNode** oldNode = hashMap->nodes;
    usize oldCapacity = hashMap->capacity;
    string_hash_map_allocate(hashMap, newCapacity);

    string_hash_map_rehash(hashMap, oldNode, oldCapacity);

    SKA_MEM_FREE(oldNode);
}

// Int
bool ska_string_hash_map_add_int(SkaStringHashMap* hashMap, const char* key, int32 value) {
    return ska_string_hash_map_add(hashMap, key, &value, sizeof(int32));
}

int32 ska_string_hash_map_get_int(SkaStringHashMap* hashMap, const char* key) {
    return *(int32*) ska_string_hash_map_get(hashMap, key);
}

// String
bool ska_string_hash_map_add_string(SkaStringHashMap* hashMap, const char* key, const char* value) {
    char* stringVal = ska_strdup(value);
    bool result = ska_string_hash_map_add(hashMap, key, stringVal, strlen(value) + 1);
    SKA_MEM_FREE(stringVal);
    return result;
}

char* ska_string_hash_map_get_string(SkaStringHashMap* hashMap, const char* key) {
    return (char*) ska_string_hash_map_get(hashMap, key);
}

//--- Iterator ---//
SkaStringHashMapIterator ska_string_hash_map_iter_create(SkaStringHashMap* hashMap) {
    // Get initial node if exists
    SkaStringHashMapNode* initialNode = NULL;
    usize initialIndex = 0;
    for (usize chain = 0; chain < hashMap->capacity; chain++) {
        SkaStringHashMapNode* node = hashMap->nodes[chain];
        if (node != NULL) {
            initialNode = node;
            initialIndex = chain + 1;
            break;
        }
    }
    usize iteratorCount = initialNode != NULL ? 1 : 0;
    SkaStringHashMapIterator iterator = { .count = iteratorCount, .end = hashMap->capacity, .index = initialIndex, .pair = initialNode };
    return iterator;
}

bool ska_string_hash_map_iter_is_valid(SkaStringHashMap* hashMap, SkaStringHashMapIterator* iterator) {
    return iterator->pair != NULL && iterator->count <= hashMap->size;
}

void ska_string_hash_map_iter_advance(SkaStringHashMap* hashMap, SkaStringHashMapIterator* iterator) {
    if (ska_string_hash_map_iter_is_valid(hashMap, iterator)) {
        if (iterator->pair->next != NULL) {
            iterator->pair = iterator->pair->next;
            iterator->count++;
            return;
        }

        // Search nodes array if there are no more linked pairs
        for (usize chain = iterator->index; chain < hashMap->capacity; chain++) {
            SkaStringHashMapNode* node = hashMap->nodes[chain];
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

void hash_map_destroy_node_string(SkaStringHashMapNode* node) {
    SKA_ASSERT(node != NULL);
    SKA_ASSERT(node->value != NULL);
    SKA_ASSERT(node->key != NULL);
    SKA_MEM_FREE(node->key);
    SKA_MEM_FREE(node->value);
    SKA_MEM_FREE(node);
}
