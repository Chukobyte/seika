#pragma once

#include "seika/math/math.h"
#include "seika/data_structures/hash_map.h"

#define SKA_SPATIAL_HASH_GRID_SPACE_ENTITY_LIMIT 32
#define SKA_SPATIAL_HASH_GRID_MAX_COLLISIONS 16

// Note: Spatial hash expects rectangles that have 0 rotation

// Contains the object id for a particular grid space
typedef struct SkaSpatialHashMapGridSpace {
    unsigned int entities[SKA_SPATIAL_HASH_GRID_SPACE_ENTITY_LIMIT];
    usize entity_count;
} SkaSpatialHashMapGridSpace;

// Contains all grid spaces an object is assigned to
typedef struct SkaSpatialHashMapGridSpacesHandle {
    usize grid_space_count;
    SkaRect2 collision_rect;
    SkaSpatialHashMapGridSpace* grid_spaces[4];
} SkaSpatialHashMapGridSpacesHandle;

// Contains a hash map of buckets that correspond to a space on a grid
typedef struct SkaSpatialHashMap {
    int cell_size;  // Cell size should be 2x the size of the largest object
    int largest_object_size; // Used to keep track of the largest object size.  If size is 8x less than cell size, we should resize spatial hash.
    SkaHashMap* grid_map; // Contains objects within a grid.
    SkaHashMap* object_to_grid_map; // Contains contains all grid spaces an object is assigned to.
    bool does_collision_data_need_updating;
} SkaSpatialHashMap;

typedef struct SkaSpatialHashMapCollisionResult {
    usize collision_count;
    unsigned int collisions[SKA_SPATIAL_HASH_GRID_MAX_COLLISIONS];
} SkaSpatialHashMapCollisionResult;

SkaSpatialHashMap* ska_spatial_hash_map_create(int32 initial_cell_size);
void ska_spatial_hash_map_destroy(SkaSpatialHashMap* hash_map);
SkaSpatialHashMapGridSpacesHandle* ska_spatial_hash_map_insert_or_update(SkaSpatialHashMap* hash_map, uint32 entity, SkaRect2* collision_rect);
void ska_spatial_hash_map_remove(SkaSpatialHashMap* hash_map, uint32 entity);
SkaSpatialHashMapGridSpacesHandle* ska_spatial_hash_map_get(SkaSpatialHashMap* hash_map, uint32 entity);
SkaSpatialHashMapCollisionResult ska_spatial_hash_map_compute_collision(SkaSpatialHashMap* hash_map, uint32 entity);
