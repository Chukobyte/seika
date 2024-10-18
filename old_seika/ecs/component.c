#include "component.h"

#include <stddef.h>

#include "seika/string.h"
#include "seika/memory.h"
#include "seika/assert.h"
#include "seika/data_structures/hash_map_string.h"

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
} ComponentArray;

static ComponentArray* component_array_create() {
    ComponentArray* componentArray = SKA_MEM_ALLOCATE(ComponentArray);
    for (unsigned int i = 0; i < SKA_ECS_MAX_COMPONENTS; i++) {
        componentArray->components[i] = NULL;
    }
    return componentArray;
}

static void component_array_initialize(ComponentArray* componentArray) {
    for (unsigned int i = 0; i < SKA_ECS_MAX_COMPONENTS; i++) {
        componentArray->components[i] = NULL;
    }
}

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
    SKA_MEM_FREE(componentArray->components[index]);
    componentArray->components[index] = NULL;
}

static void component_array_remove_all_components(ComponentArray* componentArray) {
    for (usize i = 0; i < SKA_ECS_MAX_COMPONENTS; i++) {
        component_array_remove_component(componentArray, (SkaComponentIndex)i);
    }
}

//--- Component Manager ---//
typedef struct ComponentManager {
    ComponentArray entityComponentArrays[SKA_MAX_ENTITIES];
    SkaComponentType entityComponentSignatures[SKA_MAX_ENTITIES];
} ComponentManager;

static ComponentManager* componentManager = NULL;

static SkaComponentType component_manager_translate_index_to_type(SkaComponentIndex index);

void ska_ecs_component_manager_initialize() {
    SKA_ASSERT(componentNameToTypeMap == NULL);
    componentNameToTypeMap = ska_string_hash_map_create_default_capacity();

    SKA_ASSERT_FMT(componentManager == NULL, "Component Manager is not NULL when trying to initialize");
    componentManager = SKA_MEM_ALLOCATE(ComponentManager);
}

// Assumes component data was already deleted previously
void ska_ecs_component_manager_finalize() {
    SKA_ASSERT(componentNameToTypeMap != NULL);
    SKA_STRING_HASH_MAP_FOR_EACH(componentNameToTypeMap, iter) {
        SkaStringHashMapNode* node = iter.pair;
        SkaComponentTypeInfo* typeInfo = (SkaComponentTypeInfo*)node->value;
        if (typeInfo) {
            SKA_MEM_FREE(typeInfo->name);
        }
    }
    ska_string_hash_map_destroy(componentNameToTypeMap);
    componentNameToTypeMap = NULL;
    globalComponentIndex = 0;

    SKA_ASSERT_FMT(componentManager != NULL, "Component Manager is NULL when trying to finalize...");
    SKA_MEM_FREE(componentManager);
    componentManager = NULL;
}

void* ska_ecs_component_manager_get_component(SkaEntity entity, SkaComponentIndex index) {
    void* component = component_array_get_component(&componentManager->entityComponentArrays[entity], index);
    SKA_ASSERT_FMT(component != NULL, "Entity '%d' doesn't have '%s' component!", entity, ska_ecs_component_get_component_data_index_string(index));
    return component;
}

void* ska_ecs_component_manager_get_component_unchecked(SkaEntity entity, SkaComponentIndex index) {
    return component_array_get_component(&componentManager->entityComponentArrays[entity], index);
}

void ska_ecs_component_manager_set_component(SkaEntity entity, SkaComponentIndex index, void* component) {
    component_array_set_component(&componentManager->entityComponentArrays[entity], index, component);
    // Update signature
    SkaComponentType componentSignature = ska_ecs_component_manager_get_component_signature(entity);
    componentSignature |= component_manager_translate_index_to_type(index);
    ska_ecs_component_manager_set_component_signature(entity, componentSignature);
}

void ska_ecs_component_manager_remove_component(SkaEntity entity, SkaComponentIndex index) {
    SkaComponentType componentSignature = ska_ecs_component_manager_get_component_signature(entity);
    componentSignature &= component_manager_translate_index_to_type(index);
    ska_ecs_component_manager_set_component_signature(entity, componentSignature);
    component_array_remove_component(&componentManager->entityComponentArrays[entity], index);
}

void ska_ecs_component_manager_remove_all_components(SkaEntity entity) {
    component_array_remove_all_components(&componentManager->entityComponentArrays[entity]);
    ska_ecs_component_manager_set_component_signature(entity, SKA_ECS_COMPONENT_TYPE_NONE);
}

bool ska_ecs_component_manager_has_component(SkaEntity entity, SkaComponentIndex index) {
    return component_array_has_component(&componentManager->entityComponentArrays[entity], index);
}

void ska_ecs_component_manager_set_component_signature(SkaEntity entity, SkaComponentType componentTypeSignature) {
    componentManager->entityComponentSignatures[entity] = componentTypeSignature;
}

SkaComponentType ska_ecs_component_manager_get_component_signature(SkaEntity entity) {
    return componentManager->entityComponentSignatures[entity];
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
