#if SKA_ECS

#include "component.h"

#include "seika/string.h"
#include "seika/memory.h"
#include "seika/assert.h"
#include "seika/data_structures/hash_map_string.h"
#include "seika/data_structures/array_list.h"

//--- Component ---//
static SkaComponentIndex globalComponentIndex = 0;
static SkaStringHashMap* componentNameToTypeMap = NULL;

const SkaComponentTypeInfo* ska_ecs_component_register_type(const char* name, usize componentSize) {
    SKA_ASSERT_FMT(globalComponentIndex + 1 < SKA_ECS_MAX_COMPONENTS, "Over the maximum allowed components which are '%d'", SKA_ECS_MAX_COMPONENTS);
    // Check if component already exists and return that index if it does
    const SkaComponentTypeInfo* typeInfo = (SkaComponentTypeInfo*)ska_string_hash_map_find(componentNameToTypeMap, name);
    if (typeInfo) {
        return typeInfo;
    }
    // Add new type info for component
    const SkaComponentIndex newTypeIndex = globalComponentIndex++;
    const SkaComponentTypeInfo newTypeInfo = {
        .name = ska_strdup(name),
        .type = 1 << newTypeIndex, // Bitshift for flag assignment
        .index = newTypeIndex,
        .size = componentSize
    };
    ska_string_hash_map_add(componentNameToTypeMap, name, &newTypeInfo, sizeof(SkaComponentTypeInfo));
    return ska_ecs_component_get_type_info(name, componentSize);
}

const SkaComponentTypeInfo* ska_ecs_component_get_type_info(const char* name, usize componentSize) {
    const SkaComponentTypeInfo* typeInfo = (SkaComponentTypeInfo*)ska_string_hash_map_find(componentNameToTypeMap, name);
    SKA_ASSERT(typeInfo);
    SKA_ASSERT(typeInfo->size == componentSize);
    return typeInfo;
}

const SkaComponentTypeInfo* ska_ecs_component_find_type_info(const char* name) {
    const SkaComponentTypeInfo* typeInfo = (SkaComponentTypeInfo*)ska_string_hash_map_find(componentNameToTypeMap, name);
    return typeInfo;
}

SkaComponentType ska_ecs_component_get_type_flag(const char* name, usize componentSize) {
    const SkaComponentTypeInfo* typeInfo = ska_ecs_component_get_type_info(name, componentSize);
    return typeInfo->type;
}

//--- Component Array ---//
typedef struct ComponentArray {
    void* components[SKA_ECS_MAX_COMPONENTS];
    SkaComponentType signature;
} ComponentArray;

static bool component_array_has_component(ComponentArray* componentArray, SkaComponentIndex index) {
    return componentArray->components[index] == NULL ? false : true;
}

static void* component_array_get_component(ComponentArray* componentArray, SkaComponentIndex index) {
    return componentArray->components[index];
}

static void component_array_set_component(ComponentArray* componentArray, SkaComponentIndex index, void* component) {
    componentArray->components[index] = component;
}

static void component_array_remove_component(ComponentArray* componentArray, SkaComponentIndex index) {
    SKA_FREE(componentArray->components[index]);
    componentArray->components[index] = NULL;
}

static void component_array_remove_all_components(ComponentArray* componentArray) {
    for (usize i = 0; i < SKA_ECS_MAX_COMPONENTS; i++) {
        component_array_remove_component(componentArray, (SkaComponentIndex)i);
    }
    componentArray->signature = SKA_ECS_COMPONENT_TYPE_NONE;
}

//--- Component Manager ---//
typedef struct ComponentManager {
    SkaArrayList* componentArrays;
} ComponentManager;

static ComponentManager componentManager = { .componentArrays = NULL };

static SkaComponentType component_manager_translate_index_to_type(SkaComponentIndex index);

void ska_ecs_component_manager_initialize() {
    SKA_ASSERT(componentNameToTypeMap == NULL);
    componentNameToTypeMap = ska_string_hash_map_create_default_capacity();

    SKA_ASSERT_FMT(componentManager.componentArrays == NULL, "Component Manager's component arrays are not NULL when trying to initialize");
    componentManager.componentArrays = ska_array_list_create(sizeof(ComponentArray), 1000);
}

// Assumes component data was already deleted previously
void ska_ecs_component_manager_finalize() {
    SKA_ASSERT(componentNameToTypeMap != NULL);
    SKA_STRING_HASH_MAP_FOR_EACH(componentNameToTypeMap, iter) {
        SkaStringHashMapNode* node = iter.pair;
        SkaComponentTypeInfo* typeInfo = (SkaComponentTypeInfo*)node->value;
        if (typeInfo) {
            SKA_FREE(typeInfo->name);
        }
    }
    ska_string_hash_map_destroy(componentNameToTypeMap);
    componentNameToTypeMap = NULL;
    globalComponentIndex = 0;

    SKA_ASSERT_FMT(componentManager.componentArrays != NULL, "Component Manager is NULL when trying to finalize...");
    ska_array_list_destroy(componentManager.componentArrays);
    componentManager.componentArrays = NULL;
}

void* ska_ecs_component_manager_get_component(SkaEntity entity, SkaComponentIndex index) {
    void* component = ska_ecs_component_manager_get_component_unchecked(entity, index);
    SKA_ASSERT_FMT(component != NULL, "Entity '%d' doesn't have '%s' component!", entity, ska_ecs_component_get_component_data_index_string(index));
    return component;
}

void* ska_ecs_component_manager_get_component_unchecked(SkaEntity entity, SkaComponentIndex index) {
    ComponentArray* componentArray = ska_array_list_get(componentManager.componentArrays, (usize)entity);
    return component_array_get_component(componentArray, index);
}

void ska_ecs_component_manager_set_component(SkaEntity entity, SkaComponentIndex index, void* component) {
    // Add to component array if entity exceeds size
    if ((usize)entity >= componentManager.componentArrays->size) {
        ska_array_list_push_back(componentManager.componentArrays, &entity);
    }
    ComponentArray* componentArray = ska_array_list_get(componentManager.componentArrays, (usize)entity);
    component_array_set_component(componentArray, index, component);
    componentArray->signature |= component_manager_translate_index_to_type(index);
}

void ska_ecs_component_manager_remove_component(SkaEntity entity, SkaComponentIndex index) {
    ComponentArray* componentArray = ska_array_list_get(componentManager.componentArrays, (usize)entity);
    componentArray->signature &= component_manager_translate_index_to_type(index);
    component_array_remove_component(componentArray, index);
}

void ska_ecs_component_manager_remove_all_components(SkaEntity entity) {
    ComponentArray* componentArray = ska_array_list_get(componentManager.componentArrays, (usize)entity);
    component_array_remove_all_components(componentArray);
}

bool ska_ecs_component_manager_has_component(SkaEntity entity, SkaComponentIndex index) {
    ComponentArray* componentArray = ska_array_list_get(componentManager.componentArrays, (usize)entity);
    return component_array_has_component(componentArray, index);
}

void ska_ecs_component_manager_set_component_signature(SkaEntity entity, SkaComponentType componentTypeSignature) {
    ComponentArray* componentArray = ska_array_list_get(componentManager.componentArrays, (usize)entity);
    componentArray->signature = componentTypeSignature;
}

SkaComponentType ska_ecs_component_manager_get_component_signature(SkaEntity entity) {
    const ComponentArray* componentArray = ska_array_list_get(componentManager.componentArrays, (usize)entity);
    return componentArray->signature;
}

SkaComponentType component_manager_translate_index_to_type(SkaComponentIndex index) {
    SKA_STRING_HASH_MAP_FOR_EACH(componentNameToTypeMap, iter) {
        SkaStringHashMapNode* node = iter.pair;
        const SkaComponentTypeInfo* typeInfo = (SkaComponentTypeInfo*)node->value;
        if (typeInfo->index == index) {
            return typeInfo->type;
        }
    }
    return SKA_ECS_COMPONENT_TYPE_NONE;
}

const char* ska_ecs_component_get_component_data_index_string(SkaComponentIndex index) {
    SKA_STRING_HASH_MAP_FOR_EACH(componentNameToTypeMap, iter) {
        SkaStringHashMapNode* node = iter.pair;
        const SkaComponentTypeInfo* typeInfo = (SkaComponentTypeInfo*)node->value;
        if (typeInfo->index == index) {
            return typeInfo->name;
        }
    }
    return "INVALID";
}

#endif // if SKA_ECS
