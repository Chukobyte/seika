#include "spatial_hash_map.h"

#include <stdint.h>
#include <string.h>

#include "seika/memory.h"
#include "seika/assert.h"

#define SKA_SPATIAL_HASH_NULL_ENTITY 4294967295
#define SKA_SPATIAL_HASH_MAX_POSITION_HASH 8

typedef struct PositionHashes {
    usize hashCount;
    int32 hashes[SKA_SPATIAL_HASH_MAX_POSITION_HASH];
} PositionHashes;

static void spatial_hash_map_update(SkaSpatialHashMap* hashMap, uint32 entity, SkaSpatialHashMapGridSpacesHandle* handle, SkaRect2* collisionRect);
static bool change_cell_size_if_needed(SkaSpatialHashMap* hashMap, SkaRect2* collisionRectToCheck);
static int32 spatial_hash(SkaSpatialHashMap* hashMap, SkaVector2* position);
static SkaSpatialHashMapGridSpace* get_or_create_grid_space(SkaSpatialHashMap* hashMap, int32 positionHash);
static bool link_object_by_position_hash(SkaSpatialHashMap* hashMap, SkaSpatialHashMapGridSpacesHandle* object, uint32 value, int32 positionHash, PositionHashes* hashes);
static bool unlink_object_by_entity(SkaSpatialHashMap* hashMap, SkaSpatialHashMapGridSpacesHandle* object, SkaSpatialHashMapGridSpace* gridSpace, uint32 entity);
static void unlink_all_objects_by_entity(SkaSpatialHashMap* hashMap, SkaSpatialHashMapGridSpacesHandle* object, uint32 entity);
static bool collision_result_has_entity(SkaSpatialHashMapCollisionResult* result, uint32 entity);

// Public facing functions
SkaSpatialHashMap* ska_spatial_hash_map_create(int32 initialCellSize) {
    SkaSpatialHashMap* map = SKA_MEM_ALLOCATE(SkaSpatialHashMap);
    map->cellSize = initialCellSize;
    map->largestObjectSize = initialCellSize;
    map->gridMap = ska_hash_map_create(sizeof(int32), sizeof(SkaSpatialHashMapGridSpace **), SKA_HASH_MAP_MIN_CAPACITY);
    map->objectToGridMap = ska_hash_map_create(sizeof(uint32), sizeof(SkaSpatialHashMapGridSpacesHandle **),
                                               SKA_HASH_MAP_MIN_CAPACITY);
    map->doesCollisionDataNeedUpdating = false;
    return map;
}

void ska_spatial_hash_map_destroy(SkaSpatialHashMap* hashMap) {
    // Grid map
    SKA_HASH_MAP_FOR_EACH(hashMap->gridMap, iter) {
        SkaHashMapNode* node = iter.pair;
        SkaSpatialHashMapGridSpace* gridSpace = (SkaSpatialHashMapGridSpace*) *(SkaSpatialHashMapGridSpace**) node->value;
        SKA_MEM_FREE(gridSpace);
    }
    ska_hash_map_destroy(hashMap->gridMap);
    // Grid space Handle Map
    SKA_HASH_MAP_FOR_EACH(hashMap->objectToGridMap, iter) {
        SkaHashMapNode* node = iter.pair;
        SkaSpatialHashMapGridSpacesHandle* handle = (SkaSpatialHashMapGridSpacesHandle*) *(SkaSpatialHashMapGridSpacesHandle**) node->value;
        SKA_MEM_FREE(handle);
    }
    ska_hash_map_destroy(hashMap->objectToGridMap);
    // Finally free the hashmap memory
    SKA_MEM_FREE(hashMap);
}

// The purpose of this function is to make sure that 'cellSize' is twice as big as the largest object
bool change_cell_size_if_needed(SkaSpatialHashMap* hashMap, SkaRect2* collisionRectToCheck) {
    const int32 objectMaxSize = collisionRectToCheck->h > collisionRectToCheck->w ? (int32)collisionRectToCheck->h : (int32)collisionRectToCheck->w;
    // Update largest object size of hashmap if applicable
    if (objectMaxSize > hashMap->largestObjectSize) {
        hashMap->largestObjectSize = objectMaxSize;
    }
    // Check if cell size needs to grow or shrink
    if (objectMaxSize > hashMap->cellSize * 2 || hashMap->largestObjectSize < hashMap->cellSize / 8) {
        hashMap->cellSize = objectMaxSize * 2;
        return true;
    }

    return false;
}

SkaSpatialHashMapGridSpacesHandle* ska_spatial_hash_map_insert_or_update(SkaSpatialHashMap* hashMap, uint32 entity, SkaRect2* collisionRect) {
    // Create new object handle if it doesn't exist
    if (!ska_hash_map_has(hashMap->objectToGridMap, &entity)) {
        SkaSpatialHashMapGridSpacesHandle* newHandle = SKA_MEM_ALLOCATE(SkaSpatialHashMapGridSpacesHandle);
        newHandle->gridSpaceCount = 0;
        newHandle->collisionRect = (SkaRect2) {
            0.0f, 0.0f, 0.0f, 0.0f
        };
        ska_hash_map_add(hashMap->objectToGridMap, &entity, &newHandle);
    }

    // Update cell size and rebuild map if an object is bigger than the cell size
    if (change_cell_size_if_needed(hashMap, collisionRect)) {
        // Since we have changed cell size and largest object size, rebuild spatial hash
        SKA_HASH_MAP_FOR_EACH(hashMap->objectToGridMap, iter) {
            const uint32 updatedEntity = *(uint32*) iter.pair->key;
            if (entity != updatedEntity) {
                SkaSpatialHashMapGridSpacesHandle* updatedHandle = (SkaSpatialHashMapGridSpacesHandle*) iter.pair->value;
                spatial_hash_map_update(hashMap, updatedEntity, updatedHandle, &updatedHandle->collisionRect);
            }
        }
    }

    SkaSpatialHashMapGridSpacesHandle* objectHandle = (SkaSpatialHashMapGridSpacesHandle*) *(SkaSpatialHashMapGridSpacesHandle**) ska_hash_map_get(
            hashMap->objectToGridMap, &entity);
    spatial_hash_map_update(hashMap, entity, objectHandle, collisionRect);
    return objectHandle;
}

void spatial_hash_map_update(SkaSpatialHashMap* hashMap, uint32 entity, SkaSpatialHashMapGridSpacesHandle* handle, SkaRect2* collisionRect) {
    memcpy(&handle->collisionRect, collisionRect, sizeof(SkaRect2));

    // Unlink all previous spaces and objects
    unlink_all_objects_by_entity(hashMap, handle, entity);

    // Add values to spaces and spaces to object handles (moving clockwise starting from top-left)
    PositionHashes hashes = { .hashCount = 0 };
    // Top left
    const int32 topLeftHash = spatial_hash(hashMap, &(SkaVector2) {
        collisionRect->x, collisionRect->y
    });
    link_object_by_position_hash(hashMap, handle, entity, topLeftHash, &hashes);
    // Top right
    const int32 topRightHash = spatial_hash(hashMap, &(SkaVector2) {
        collisionRect->x + collisionRect->w, collisionRect->y
    });
    link_object_by_position_hash(hashMap, handle, entity, topRightHash, &hashes);
    // Bottom Left
    const int32 bottomLeftHash = spatial_hash(hashMap, &(SkaVector2) {
        collisionRect->x, collisionRect->y + collisionRect->h
    });
    link_object_by_position_hash(hashMap, handle, entity, bottomLeftHash, &hashes);
    // Bottom Right
    const int32 bottomRightHash = spatial_hash(hashMap, &(SkaVector2) {
        collisionRect->x + collisionRect->w, collisionRect->y + collisionRect->h
    });
    link_object_by_position_hash(hashMap, handle, entity, bottomRightHash, &hashes);
}

void ska_spatial_hash_map_remove(SkaSpatialHashMap* hashMap, uint32 entity) {
    if (!ska_hash_map_has(hashMap->objectToGridMap, &entity)) {
        return;
    }
    SkaSpatialHashMapGridSpacesHandle* objectHandle = (SkaSpatialHashMapGridSpacesHandle*) *(SkaSpatialHashMapGridSpacesHandle**) ska_hash_map_get(
            hashMap->objectToGridMap, &entity);
    unlink_all_objects_by_entity(hashMap, objectHandle, entity);
    ska_hash_map_erase(hashMap->objectToGridMap, &entity);
    // TODO: Use something more efficient than looping through the entire hashmap to find the largest object size
    const int32 MaxObjectSize = objectHandle->collisionRect.h > objectHandle->collisionRect.w ? (int32)objectHandle->collisionRect.h : (int32)objectHandle->collisionRect.w;
    if (MaxObjectSize == hashMap->largestObjectSize) {
        int32 foundLargestObjectSize = -1;
        SKA_HASH_MAP_FOR_EACH(hashMap->objectToGridMap, iter) {
            SkaSpatialHashMapGridSpacesHandle* nodeObjectHandle = (SkaSpatialHashMapGridSpacesHandle*) iter.pair->value;
            const int32 nodeMaxObjectSize = nodeObjectHandle->collisionRect.h > nodeObjectHandle->collisionRect.w ? (int32)nodeObjectHandle->collisionRect.h : (int32)nodeObjectHandle->collisionRect.w;
            // Early out if we find another object with the same size
            if (nodeMaxObjectSize == hashMap->largestObjectSize) {
                foundLargestObjectSize = -1;
                break;
            } else if(nodeMaxObjectSize > foundLargestObjectSize) {
                foundLargestObjectSize = nodeMaxObjectSize;
            }
        }
        if (foundLargestObjectSize > 0) {
            hashMap->largestObjectSize = foundLargestObjectSize;
        }
    }
    SKA_MEM_FREE(objectHandle);
}

SkaSpatialHashMapGridSpacesHandle* ska_spatial_hash_map_get(SkaSpatialHashMap* hashMap, uint32 entity) {
    if (ska_hash_map_has(hashMap->objectToGridMap, &entity)) {
        return (SkaSpatialHashMapGridSpacesHandle*) *(SkaSpatialHashMapGridSpacesHandle**) ska_hash_map_get(
                hashMap->objectToGridMap, &entity);
    }
    return NULL;
}

SkaSpatialHashMapCollisionResult ska_spatial_hash_map_compute_collision(SkaSpatialHashMap* hashMap, uint32 entity) {
    SkaSpatialHashMapCollisionResult result = { .collisionCount = 0 };
    void* objectHandlePtr = ska_hash_map_get(hashMap->objectToGridMap, &entity);
    // Early out if object not in spatial hash map
    if (objectHandlePtr == NULL) {
        return result;
    }
    SkaSpatialHashMapGridSpacesHandle* objectHandle = (SkaSpatialHashMapGridSpacesHandle*) *(SkaSpatialHashMapGridSpacesHandle**) objectHandlePtr;
    for (usize i = 0; i < objectHandle->gridSpaceCount; i++) {
        SkaSpatialHashMapGridSpace* gridSpace = objectHandle->gridSpaces[i];
        for (usize j = 0; j < gridSpace->entityCount; j++) {
            uint32 entityToCollide = gridSpace->entities[j];
            if (entity != entityToCollide && !collision_result_has_entity(&result, entityToCollide)) {
                SkaSpatialHashMapGridSpacesHandle* entityToCollideObjectHandle = (SkaSpatialHashMapGridSpacesHandle*) *(SkaSpatialHashMapGridSpacesHandle**) ska_hash_map_get(
                        hashMap->objectToGridMap, &entityToCollide);
                // Now that we have passed all checks, actually check collision
                if (se_rect2_does_rectangles_overlap(&objectHandle->collisionRect, &entityToCollideObjectHandle->collisionRect)) {
                    SKA_ASSERT_FMT(result.collisionCount + 1 <= SKA_SPATIAL_HASH_GRID_MAX_COLLISIONS, "At limit of collisions '%d', consider increasing 'SE_SPATIAL_HASH_GRID_MAX_COLLISIONS'", SKA_SPATIAL_HASH_GRID_MAX_COLLISIONS);
                    result.collisions[result.collisionCount++] = entityToCollide;
                }
            }
        }
    }
    return result;
}

// Internal Functions
int32 spatial_hash(SkaSpatialHashMap* hashMap, SkaVector2* position) {
    const int32 x = (int32) position->x / hashMap->cellSize;
    const int32 y = (int32) position->y / hashMap->cellSize;
    const int32 hash = ((x * x) ^ (y * y)) % INT32_MAX;
    return hash;
}

SkaSpatialHashMapGridSpace* get_or_create_grid_space(SkaSpatialHashMap* hashMap, int32 positionHash) {
    if (!ska_hash_map_has(hashMap->gridMap, &positionHash)) {
        SkaSpatialHashMapGridSpace* newGridSpace = SKA_MEM_ALLOCATE(SkaSpatialHashMapGridSpace);
        newGridSpace->entityCount = 0;
        ska_hash_map_add(hashMap->gridMap, &positionHash, &newGridSpace);
    }
    SkaSpatialHashMapGridSpace* gridSpace = (SkaSpatialHashMapGridSpace*) *(SkaSpatialHashMapGridSpace**) ska_hash_map_get(
            hashMap->gridMap, &positionHash);
    return gridSpace;
}

bool link_object_by_position_hash(SkaSpatialHashMap* hashMap, SkaSpatialHashMapGridSpacesHandle* object, uint32 value, int32 positionHash, PositionHashes* hashes) {
    // Exit if position hash exists
    for (usize i = 0; i < hashes->hashCount; i++) {
        if (positionHash == hashes->hashes[i]) {
            return false;
        }
    }
    // Link
    SkaSpatialHashMapGridSpace* gridSpace = get_or_create_grid_space(hashMap, positionHash);
    gridSpace->entities[gridSpace->entityCount++] = value;
    object->gridSpaces[object->gridSpaceCount++] = gridSpace;
    SKA_ASSERT_FMT(hashes->hashCount + 1 < SKA_SPATIAL_HASH_MAX_POSITION_HASH,
                  "Current hash count for value '%d' exceeds 'SE_SPATIAL_HASH_MAX_POSITION_HASH (%d)', consider increasing SE_SPATIAL_HASH_MAX_POSITION_HASH!");
    hashes->hashes[hashes->hashCount++] = positionHash;
    return true;
}

bool unlink_object_by_entity(SkaSpatialHashMap* hashMap, SkaSpatialHashMapGridSpacesHandle* object, SkaSpatialHashMapGridSpace* gridSpace, uint32 entity) {
    bool objectUnlinked = false;
    for (usize i = 0; i < gridSpace->entityCount; i++) {
        if (entity == gridSpace->entities[i] || gridSpace->entities[i] == SKA_SPATIAL_HASH_NULL_ENTITY) {
            if (i + 1 < gridSpace->entityCount) {
                gridSpace->entities[i] = gridSpace->entities[i + 1];
                gridSpace->entities[i + 1] = SKA_SPATIAL_HASH_NULL_ENTITY;
            }
            objectUnlinked = true;
        }
    }
    if (objectUnlinked) {
        object->gridSpaceCount--;
        gridSpace->entityCount--;
    }
    return objectUnlinked;
}

void unlink_all_objects_by_entity(SkaSpatialHashMap* hashMap, SkaSpatialHashMapGridSpacesHandle* object, uint32 entity) {
    const usize numberOfSpaces = object->gridSpaceCount;
    for (usize i = 0; i < numberOfSpaces; i++) {
        unlink_object_by_entity(hashMap, object, object->gridSpaces[i], entity);
    }
}

bool collision_result_has_entity(SkaSpatialHashMapCollisionResult* result, uint32 entity) {
    for (usize i = 0; i < result->collisionCount; i++) {
        if (entity == result->collisions[i]) {
            return true;
        }
    }
    return false;
}
