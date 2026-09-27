#if SKA_ECS

#include "component.h"

#include <string.h>

#include "seika/string.h"
#include "seika/memory.h"
#include "seika/assert.h"
#include "seika/flag_utils.h"
#include "seika/data_structures/array_list.h"
#include "seika/data_structures/array_list_template.h"
#include "seika/data_structures/hash_map_template.h"

//--- Typed containers ---//
SKA_ARRAY_LIST_DECLARE(SkaComponentSignatureList, SkaComponentType)
SKA_ARRAY_LIST_IMPLEMENT(SkaComponentSignatureList, SkaComponentType)

SKA_HASH_MAP_DECLARE(ComponentTypeInfoMap, const char*, SkaComponentTypeInfo)
SKA_HASH_MAP_IMPLEMENT(ComponentTypeInfoMap, const char*, SkaComponentTypeInfo)

static usize component_type_name_hash(const char* const* key) {
    usize hash = 5381;
    for (const unsigned char* byte = (const unsigned char*)*key; *byte != '\0'; byte++) {
        hash = ((hash << 5) + hash) ^ *byte;
    }
    return hash;
}

static int32 component_type_name_compare(const char* const* a, const char* const* b) {
    return strcmp(*a, *b);
}

//--- Component Manager ---//
// SoA storage: one contiguous, entity-indexed pool per component type. Pools are byte-generic
// since component types are registered dynamically at runtime (by name + size), so the engine
// can't know their concrete C types at compile time. The signature list's element type is known
// at compile time, so it uses the typed template container instead.
typedef struct ComponentManager {
    SkaArrayList pools[SKA_ECS_MAX_COMPONENTS];
    SkaComponentSignatureList signatures;
} ComponentManager;

static ComponentManager component_manager = {0};

static void component_manager_init_pool(SkaComponentIndex index, usize component_size) {
    component_manager.pools[index] = ska_array_list_init(component_size, 16);
}

//--- Component ---//
static SkaComponentIndex global_component_index = 0;
static ComponentTypeInfoMap* component_name_to_type_map = NULL;
// Caches populated once per type at registration, for O(1) index -> type/name lookups
static SkaComponentType type_by_index[SKA_ECS_MAX_COMPONENTS];
static const char* name_by_index[SKA_ECS_MAX_COMPONENTS];

const SkaComponentTypeInfo* ska_ecs_component_register_type(const char* name, usize component_size) {
    SKA_ASSERT_FMT(global_component_index + 1 < SKA_ECS_MAX_COMPONENTS, "Over the maximum allowed components which are '%d'", SKA_ECS_MAX_COMPONENTS);
    // Check if component already exists and return that index if it does
    const SkaComponentTypeInfo* type_info = ComponentTypeInfoMap_get(component_name_to_type_map, name);
    if (type_info) {
        return type_info;
    }
    // Add new type info for component
    const SkaComponentIndex new_type_index = global_component_index++;
    const SkaComponentTypeInfo new_type_info = {
        .name = ska_strdup(name),
        .type = 1 << new_type_index, // Bitshift for flag assignment
        .index = new_type_index,
        .size = component_size
    };
    type_by_index[new_type_index] = new_type_info.type;
    name_by_index[new_type_index] = new_type_info.name;
    // Key is the strdup'd name so its lifetime matches the entry (freed once, in finalize)
    ComponentTypeInfoMap_add(component_name_to_type_map, new_type_info.name, new_type_info);
    component_manager_init_pool(new_type_index, component_size);
    return ComponentTypeInfoMap_get(component_name_to_type_map, name);
}

const SkaComponentTypeInfo* ska_ecs_component_get_type_info(const char* name, usize component_size) {
    const SkaComponentTypeInfo* type_info = ComponentTypeInfoMap_get(component_name_to_type_map, name);
    SKA_ASSERT(type_info);
    SKA_ASSERT(type_info->size == component_size);
    return type_info;
}

const SkaComponentTypeInfo* ska_ecs_component_find_type_info(const char* name) {
    return ComponentTypeInfoMap_get(component_name_to_type_map, name);
}

SkaComponentType ska_ecs_component_get_type_flag(const char* name, usize component_size) {
    const SkaComponentTypeInfo* type_info = ska_ecs_component_get_type_info(name, component_size);
    return type_info->type;
}

void ska_ecs_component_manager_initialize() {
    SKA_ASSERT(component_name_to_type_map == NULL);
    component_name_to_type_map = ComponentTypeInfoMap_init(16);
    component_name_to_type_map->hashFunc = component_type_name_hash;
    component_name_to_type_map->compareFunc = component_type_name_compare;

    component_manager.signatures = SkaComponentSignatureList_init(1000);
}

// Assumes component data was already deleted previously
void ska_ecs_component_manager_finalize() {
    SKA_ASSERT(component_name_to_type_map != NULL);
    SKA_HASH_MAP_T_FOR_EACH(component_name_to_type_map, ComponentTypeInfoMap, iter) {
        SKA_FREE(iter.pair->value.name);
    }
    ComponentTypeInfoMap_deinit(component_name_to_type_map);
    component_name_to_type_map = NULL;

    for (usize i = 0; i < global_component_index; i++) {
        ska_array_list_deinit(&component_manager.pools[i]);
    }
    global_component_index = 0;

    SkaComponentSignatureList_deinit(&component_manager.signatures);
}

bool ska_ecs_component_manager_has_component(SkaEntity entity, SkaComponentIndex index) {
    const SkaComponentType signature = ska_ecs_component_manager_get_component_signature(entity);
    return SKA_FLAG_CONTAINS(signature, type_by_index[index]);
}

void* ska_ecs_component_manager_get_component(SkaEntity entity, SkaComponentIndex index) {
    void* component = ska_ecs_component_manager_get_component_unchecked(entity, index);
    SKA_ASSERT_FMT(component != NULL, "Entity '%d' doesn't have '%s' component!", entity, ska_ecs_component_get_component_data_index_string(index));
    return component;
}

void* ska_ecs_component_manager_get_component_unchecked(SkaEntity entity, SkaComponentIndex index) {
    if (!ska_ecs_component_manager_has_component(entity, index)) {
        return NULL;
    }
    return ska_array_list_get(&component_manager.pools[index], (usize)entity);
}

void ska_ecs_component_manager_set_component(SkaEntity entity, SkaComponentIndex index, void* component) {
    ska_ecs_component_manager_reserve(entity);
    SkaArrayList* pool = &component_manager.pools[index];
    ska_array_list_resize_zeroed(pool, (usize)entity + 1);
    memcpy(ska_array_list_get(pool, (usize)entity), component, pool->value_size);
    SkaComponentType* signature = SkaComponentSignatureList_get(&component_manager.signatures, (usize)entity);
    *signature |= type_by_index[index];
}

void ska_ecs_component_manager_remove_component(SkaEntity entity, SkaComponentIndex index) {
    SkaComponentType* signature = SkaComponentSignatureList_get(&component_manager.signatures, (usize)entity);
    SKA_REMOVE_FLAGS(*signature, type_by_index[index]);
}

void ska_ecs_component_manager_remove_all_components(SkaEntity entity) {
    SkaComponentType* signature = SkaComponentSignatureList_get(&component_manager.signatures, (usize)entity);
    *signature = SKA_ECS_COMPONENT_TYPE_NONE;
}

void ska_ecs_component_manager_set_component_signature(SkaEntity entity, SkaComponentType component_type_signature) {
    SkaComponentType* signature = SkaComponentSignatureList_get(&component_manager.signatures, (usize)entity);
    *signature = component_type_signature;
}

SkaComponentType ska_ecs_component_manager_get_component_signature(SkaEntity entity) {
    return *SkaComponentSignatureList_get(&component_manager.signatures, (usize)entity);
}

void ska_ecs_component_manager_reserve(SkaEntity last_entity) {
    SkaComponentSignatureList_resize_zeroed(&component_manager.signatures, (usize)last_entity + 1);
}

const char* ska_ecs_component_get_component_data_index_string(SkaComponentIndex index) {
    return index < global_component_index ? name_by_index[index] : "INVALID";
}

#endif // if SKA_ECS
