#include "spatial_hash_map.h"

#include <stdint.h>
#include <string.h>

#include "seika/memory.h"
#include "seika/assert.h"

#define SKA_SPATIAL_HASH_NULL_ENTITY 4294967295
#define SKA_SPATIAL_HASH_MAX_POSITION_HASH 8

typedef struct PositionHashes {
  usize hash_count;
  int32 hashes[SKA_SPATIAL_HASH_MAX_POSITION_HASH];
} PositionHashes;

static void spatial_hash_map_update(SkaSpatialHashMap *hash_map, uint32 entity,
                                    SkaSpatialHashMapGridSpacesHandle *handle,
                                    SkaRect2 *collision_rect);
static bool change_cell_size_if_needed(SkaSpatialHashMap *hash_map,
                                       SkaRect2 *collision_rect_to_check);
static int32 spatial_hash(SkaSpatialHashMap *hash_map, SkaVector2 *position);
static SkaSpatialHashMapGridSpace *
get_or_create_grid_space(SkaSpatialHashMap *hash_map, int32 position_hash);
static bool link_object_by_position_hash(
    SkaSpatialHashMap *hash_map, SkaSpatialHashMapGridSpacesHandle *object,
    uint32 value, int32 position_hash, PositionHashes *hashes);
static bool unlink_object_by_entity(SkaSpatialHashMap *hash_map,
                                    SkaSpatialHashMapGridSpacesHandle *object,
                                    SkaSpatialHashMapGridSpace *grid_space,
                                    uint32 entity);
static void
unlink_all_objects_by_entity(SkaSpatialHashMap *hash_map,
                             SkaSpatialHashMapGridSpacesHandle *object,
                             uint32 entity);
static bool collision_result_has_entity(SkaSpatialHashMapCollisionResult* result, uint32 entity);

// Public facing functions
SkaSpatialHashMap *ska_spatial_hash_map_create(int32 initial_cell_size) {
  SkaSpatialHashMap *map = SKA_ALLOC(SkaSpatialHashMap);
  map->cell_size = initial_cell_size;
  map->largest_object_size = initial_cell_size;
  map->grid_map =
      ska_hash_map_init(sizeof(int32), sizeof(SkaSpatialHashMapGridSpace **),
                        SKA_HASH_MAP_MIN_CAPACITY);
  map->object_to_grid_map = ska_hash_map_init(
      sizeof(uint32), sizeof(SkaSpatialHashMapGridSpacesHandle **),
      SKA_HASH_MAP_MIN_CAPACITY);
  map->does_collision_data_need_updating = false;
  return map;
}

void ska_spatial_hash_map_destroy(SkaSpatialHashMap *hash_map) {
  // Grid map
  SKA_HASH_MAP_FOR_EACH(hash_map->grid_map, iter) {
    SkaHashMapNode *node = iter.pair;
    SkaSpatialHashMapGridSpace *grid_space =
        (SkaSpatialHashMapGridSpace *)*(
            SkaSpatialHashMapGridSpace **)node->value;
    SKA_FREE(grid_space);
  }
  ska_hash_map_deinit(hash_map->grid_map);
  // Grid space Handle Map
  SKA_HASH_MAP_FOR_EACH(hash_map->object_to_grid_map, iter) {
    SkaHashMapNode *node = iter.pair;
    SkaSpatialHashMapGridSpacesHandle *handle =
        (SkaSpatialHashMapGridSpacesHandle *)*(
            SkaSpatialHashMapGridSpacesHandle **)node->value;
    SKA_FREE(handle);
  }
  ska_hash_map_deinit(hash_map->object_to_grid_map);
  // Finally free the hashmap memory
  SKA_FREE(hash_map);
}

// The purpose of this function is to make sure that 'cell_size' is twice as big as the largest object
bool change_cell_size_if_needed(SkaSpatialHashMap *hash_map,
                                SkaRect2 *collision_rect_to_check) {
  const int32 object_max_size =
      collision_rect_to_check->h > collision_rect_to_check->w
          ? (int32)collision_rect_to_check->h
          : (int32)collision_rect_to_check->w;
  // Update largest object size of hashmap if applicable
  if (object_max_size > hash_map->largest_object_size) {
    hash_map->largest_object_size = object_max_size;
  }
  // Check if cell size needs to grow or shrink
  if (object_max_size > hash_map->cell_size * 2 ||
      hash_map->largest_object_size < hash_map->cell_size / 8) {
    hash_map->cell_size = object_max_size * 2;
    return true;
  }

  return false;
}

SkaSpatialHashMapGridSpacesHandle *
ska_spatial_hash_map_insert_or_update(SkaSpatialHashMap *hash_map,
                                      uint32 entity, SkaRect2 *collision_rect) {
  // Create new object handle if it doesn't exist
  if (!ska_hash_map_has(hash_map->object_to_grid_map, &entity)) {
    SkaSpatialHashMapGridSpacesHandle *new_handle =
        SKA_ALLOC(SkaSpatialHashMapGridSpacesHandle);
    new_handle->grid_space_count = 0;
    new_handle->collision_rect = (SkaRect2){0.0f, 0.0f, 0.0f, 0.0f};
    ska_hash_map_add(hash_map->object_to_grid_map, &entity, &new_handle);
  }

  // Update cell size and rebuild map if an object is bigger than the cell size
  if (change_cell_size_if_needed(hash_map, collision_rect)) {
    // Since we have changed cell size and largest object size, rebuild spatial
    // hash
    SKA_HASH_MAP_FOR_EACH(hash_map->object_to_grid_map, iter) {
      const uint32 updated_entity = *(uint32 *)iter.pair->key;
      if (entity != updated_entity) {
        SkaSpatialHashMapGridSpacesHandle *updated_handle =
            (SkaSpatialHashMapGridSpacesHandle *)iter.pair->value;
        spatial_hash_map_update(hash_map, updated_entity, updated_handle,
                                &updated_handle->collision_rect);
      }
    }
  }

  SkaSpatialHashMapGridSpacesHandle *object_handle =
      (SkaSpatialHashMapGridSpacesHandle *)*(
          SkaSpatialHashMapGridSpacesHandle **)
          ska_hash_map_get(hash_map->object_to_grid_map, &entity);
  spatial_hash_map_update(hash_map, entity, object_handle, collision_rect);
  return object_handle;
}

void spatial_hash_map_update(SkaSpatialHashMap *hash_map, uint32 entity,
                             SkaSpatialHashMapGridSpacesHandle *handle,
                             SkaRect2 *collision_rect) {
  memcpy(&handle->collision_rect, collision_rect, sizeof(SkaRect2));

  // Unlink all previous spaces and objects
  unlink_all_objects_by_entity(hash_map, handle, entity);

  // Add values to spaces and spaces to object handles (moving clockwise
  // starting from top-left)
  PositionHashes hashes = {.hash_count = 0};
  // Top left
  const int32 top_left_hash = spatial_hash(
      hash_map, &(SkaVector2){collision_rect->x, collision_rect->y});
  link_object_by_position_hash(hash_map, handle, entity, top_left_hash,
                               &hashes);
  // Top right
  const int32 top_right_hash = spatial_hash(
      hash_map,
      &(SkaVector2){collision_rect->x + collision_rect->w, collision_rect->y});
  link_object_by_position_hash(hash_map, handle, entity, top_right_hash,
                               &hashes);
  // Bottom Left
  const int32 bottom_left_hash = spatial_hash(
      hash_map,
      &(SkaVector2){collision_rect->x, collision_rect->y + collision_rect->h});
  link_object_by_position_hash(hash_map, handle, entity, bottom_left_hash,
                               &hashes);
  // Bottom Right
  const int32 bottom_right_hash = spatial_hash(
      hash_map, &(SkaVector2){collision_rect->x + collision_rect->w,
                              collision_rect->y + collision_rect->h});
  link_object_by_position_hash(hash_map, handle, entity, bottom_right_hash,
                               &hashes);
}

void ska_spatial_hash_map_remove(SkaSpatialHashMap *hash_map, uint32 entity) {
  if (!ska_hash_map_has(hash_map->object_to_grid_map, &entity)) {
    return;
  }
  SkaSpatialHashMapGridSpacesHandle *object_handle =
      (SkaSpatialHashMapGridSpacesHandle *)*(
          SkaSpatialHashMapGridSpacesHandle **)
          ska_hash_map_get(hash_map->object_to_grid_map, &entity);
  unlink_all_objects_by_entity(hash_map, object_handle, entity);
  ska_hash_map_erase(hash_map->object_to_grid_map, &entity);
  // TODO: Use something more efficient than looping through the entire hashmap
  // to find the largest object size
  const int32 max_object_size =
      object_handle->collision_rect.h > object_handle->collision_rect.w
          ? (int32)object_handle->collision_rect.h
          : (int32)object_handle->collision_rect.w;
  if (max_object_size == hash_map->largest_object_size) {
    int32 found_largest_object_size = -1;
    SKA_HASH_MAP_FOR_EACH(hash_map->object_to_grid_map, iter) {
      SkaSpatialHashMapGridSpacesHandle *node_object_handle =
          (SkaSpatialHashMapGridSpacesHandle *)iter.pair->value;
      const int32 node_max_object_size =
          node_object_handle->collision_rect.h >
                  node_object_handle->collision_rect.w
              ? (int32)node_object_handle->collision_rect.h
              : (int32)node_object_handle->collision_rect.w;
      // Early out if we find another object with the same size
      if (node_max_object_size == hash_map->largest_object_size) {
        found_largest_object_size = -1;
        break;
      } else if (node_max_object_size > found_largest_object_size) {
        found_largest_object_size = node_max_object_size;
      }
    }
    if (found_largest_object_size > 0) {
      hash_map->largest_object_size = found_largest_object_size;
    }
  }
  SKA_FREE(object_handle);
}

SkaSpatialHashMapGridSpacesHandle *
ska_spatial_hash_map_get(SkaSpatialHashMap *hash_map, uint32 entity) {
  if (ska_hash_map_has(hash_map->object_to_grid_map, &entity)) {
    return (SkaSpatialHashMapGridSpacesHandle *)*(
        SkaSpatialHashMapGridSpacesHandle **)
        ska_hash_map_get(hash_map->object_to_grid_map, &entity);
  }
  return NULL;
}

SkaSpatialHashMapCollisionResult
ska_spatial_hash_map_compute_collision(SkaSpatialHashMap *hash_map,
                                       uint32 entity) {
  SkaSpatialHashMapCollisionResult result = {.collision_count = 0};
  void *object_handle_ptr =
      ska_hash_map_get(hash_map->object_to_grid_map, &entity);
  // Early out if object not in spatial hash map
  if (object_handle_ptr == NULL) {
    return result;
  }
  SkaSpatialHashMapGridSpacesHandle *object_handle =
      (SkaSpatialHashMapGridSpacesHandle *)*(
          SkaSpatialHashMapGridSpacesHandle **)object_handle_ptr;
  for (usize i = 0; i < object_handle->grid_space_count; i++) {
    SkaSpatialHashMapGridSpace *grid_space = object_handle->grid_spaces[i];
    for (usize j = 0; j < grid_space->entity_count; j++) {
      uint32 entity_to_collide = grid_space->entities[j];
      if (entity != entity_to_collide &&
          !collision_result_has_entity(&result, entity_to_collide)) {
        SkaSpatialHashMapGridSpacesHandle *entity_to_collide_object_handle =
            (SkaSpatialHashMapGridSpacesHandle *)*(
                SkaSpatialHashMapGridSpacesHandle **)
                ska_hash_map_get(hash_map->object_to_grid_map, &entity_to_collide);
        // Now that we have passed all checks, actually check collision
        if (se_rect2_does_rectangles_overlap(
                &object_handle->collision_rect,
                &entity_to_collide_object_handle->collision_rect)) {
          SKA_ASSERT_FMT(result.collision_count + 1 <=
                             SKA_SPATIAL_HASH_GRID_MAX_COLLISIONS,
                         "At limit of collisions '%d', consider increasing "
                         "'SE_SPATIAL_HASH_GRID_MAX_COLLISIONS'",
                         SKA_SPATIAL_HASH_GRID_MAX_COLLISIONS);
          result.collisions[result.collision_count++] = entity_to_collide;
        }
      }
    }
  }
  return result;
}

// Internal Functions
int32 spatial_hash(SkaSpatialHashMap *hash_map, SkaVector2 *position) {
  const int32 x = (int32)position->x / hash_map->cell_size;
  const int32 y = (int32)position->y / hash_map->cell_size;
  const int32 hash = ((x * x) ^ (y * y)) % INT32_MAX;
  return hash;
}

SkaSpatialHashMapGridSpace *
get_or_create_grid_space(SkaSpatialHashMap *hash_map, int32 position_hash) {
  if (!ska_hash_map_has(hash_map->grid_map, &position_hash)) {
    SkaSpatialHashMapGridSpace *new_grid_space =
        SKA_ALLOC(SkaSpatialHashMapGridSpace);
    new_grid_space->entity_count = 0;
    ska_hash_map_add(hash_map->grid_map, &position_hash, &new_grid_space);
  }
  SkaSpatialHashMapGridSpace *grid_space = (SkaSpatialHashMapGridSpace *)*(
      SkaSpatialHashMapGridSpace **)ska_hash_map_get(hash_map->grid_map,
                                                     &position_hash);
  return grid_space;
}

bool link_object_by_position_hash(SkaSpatialHashMap *hash_map,
                                  SkaSpatialHashMapGridSpacesHandle *object,
                                  uint32 value, int32 position_hash,
                                  PositionHashes *hashes) {
  // Exit if position hash exists
  for (usize i = 0; i < hashes->hash_count; i++) {
    if (position_hash == hashes->hashes[i]) {
      return false;
    }
  }
  // Link
  SkaSpatialHashMapGridSpace *grid_space =
      get_or_create_grid_space(hash_map, position_hash);
  grid_space->entities[grid_space->entity_count++] = value;
  object->grid_spaces[object->grid_space_count++] = grid_space;
  SKA_ASSERT_FMT(hashes->hash_count + 1 < SKA_SPATIAL_HASH_MAX_POSITION_HASH,
                 "Current hash count for value '%d' exceeds "
                 "'SE_SPATIAL_HASH_MAX_POSITION_HASH (%d)', consider "
                 "increasing SE_SPATIAL_HASH_MAX_POSITION_HASH!");
  hashes->hashes[hashes->hash_count++] = position_hash;
  return true;
}

bool unlink_object_by_entity(SkaSpatialHashMap *hash_map,
                             SkaSpatialHashMapGridSpacesHandle *object,
                             SkaSpatialHashMapGridSpace *grid_space,
                             uint32 entity) {
  bool object_unlinked = false;
  for (usize i = 0; i < grid_space->entity_count; i++) {
    if (entity == grid_space->entities[i] ||
        grid_space->entities[i] == SKA_SPATIAL_HASH_NULL_ENTITY) {
      if (i + 1 < grid_space->entity_count) {
        grid_space->entities[i] = grid_space->entities[i + 1];
        grid_space->entities[i + 1] = SKA_SPATIAL_HASH_NULL_ENTITY;
      }
      object_unlinked = true;
    }
  }
  if (object_unlinked) {
    object->grid_space_count--;
    grid_space->entity_count--;
  }
  return object_unlinked;
}

void unlink_all_objects_by_entity(SkaSpatialHashMap *hash_map,
                                  SkaSpatialHashMapGridSpacesHandle *object,
                                  uint32 entity) {
  const usize number_of_spaces = object->grid_space_count;
  for (usize i = 0; i < number_of_spaces; i++) {
    unlink_object_by_entity(hash_map, object, object->grid_spaces[i], entity);
  }
}

bool collision_result_has_entity(SkaSpatialHashMapCollisionResult* result, uint32 entity) {
    for (usize i = 0; i < result->collision_count; i++) {
        if (entity == result->collisions[i]) {
            return true;
        }
    }
    return false;
}
