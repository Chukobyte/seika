#pragma once

#include "seika/math/math.h"
#include "hash_map.h"

#define SKA_SPATIAL_HASH_GRID_SPACE_ENTITY_LIMIT 32
#define SKA_SPATIAL_HASH_GRID_MAX_COLLISIONS 16

// Note: Spatial hash expects rectangles that have 0 rotation

// Contains the object id for a particular grid space
typedef struct SkaSpatialHashMapGridSpace {
    unsigned int entities[SKA_SPATIAL_HASH_GRID_SPACE_ENTITY_LIMIT];
    usize entityCount;
} SkaSpatialHashMapGridSpace;

// Contains all grid spaces an object is assigned to
typedef struct SkaSpatialHashMapGridSpacesHandle {
    usize gridSpaceCount;
    SkaRect2 collisionRect;
    SkaSpatialHashMapGridSpace* gridSpaces[4];
} SkaSpatialHashMapGridSpacesHandle;

// Contains a hash map of buckets that correspond to a space on a grid
typedef struct SkaSpatialHashMap {
    int cellSize;  // Cell size should be 2x the size of the largest object
    int largestObjectSize; // Used to keep track of the largest object size.  If size is 8x less than cell size, we should resize spatial hash.
    SkaHashMap* gridMap; // Contains objects within a grid.
    SkaHashMap* objectToGridMap; // Contains contains all grid spaces an object is assigned to.
    bool doesCollisionDataNeedUpdating;
} SkaSpatialHashMap;

typedef struct SkaSpatialHashMapCollisionResult {
    usize collisionCount;
    unsigned int collisions[SKA_SPATIAL_HASH_GRID_MAX_COLLISIONS];
} SkaSpatialHashMapCollisionResult;

SkaSpatialHashMap* ska_spatial_hash_map_create(int32 initialCellSize);
void ska_spatial_hash_map_destroy(SkaSpatialHashMap* hashMap);
SkaSpatialHashMapGridSpacesHandle* ska_spatial_hash_map_insert_or_update(SkaSpatialHashMap* hashMap, uint32 entity, SkaRect2* collisionRect);
void ska_spatial_hash_map_remove(SkaSpatialHashMap* hashMap, uint32 entity);
SkaSpatialHashMapGridSpacesHandle* ska_spatial_hash_map_get(SkaSpatialHashMap* hashMap, uint32 entity);
SkaSpatialHashMapCollisionResult ska_spatial_hash_map_compute_collision(SkaSpatialHashMap* hashMap, uint32 entity);
