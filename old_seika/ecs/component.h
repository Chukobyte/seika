#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdbool.h>

#include "entity.h"

#define SKA_ECS_MAX_COMPONENTS 32
#define SKA_ECS_COMPONENT_TYPE_NONE ((SkaComponentType)0)

#define SKA_ECS_REGISTER_COMPONENT(ComponentType) \
ska_ecs_component_register_type(#ComponentType, sizeof(ComponentType))

#define SKA_ECS_COMPONENT_TYPE_INFO(ComponentType) \
ska_ecs_component_get_type_info(#ComponentType, sizeof(ComponentType))

#define SKA_ECS_COMPONENT_TYPE_FLAG(ComponentType) \
ska_ecs_component_get_type_flag(#ComponentType, sizeof(ComponentType))

typedef uint32 SkaComponentIndex;
typedef uint32 SkaComponentType;

typedef struct SkaComponentTypeInfo {
    char* name;
    SkaComponentType type;
    SkaComponentIndex index;
    usize size;
} SkaComponentTypeInfo;



const SkaComponentTypeInfo* ska_ecs_component_register_type(const char* name, usize componentSize);
const SkaComponentTypeInfo* ska_ecs_component_get_type_info(const char* name, usize componentSize);
const SkaComponentTypeInfo* ska_ecs_component_find_type_info(const char* name);
SkaComponentType ska_ecs_component_get_type_flag(const char* name, usize componentSize);

// --- Component Manager --- //
void ska_ecs_component_manager_initialize();
void ska_ecs_component_manager_finalize();
void* ska_ecs_component_manager_get_component(SkaEntity entity, SkaComponentIndex index);
void* ska_ecs_component_manager_get_component_unchecked(SkaEntity entity, SkaComponentIndex index); // No check, will probably consolidate later...
void ska_ecs_component_manager_set_component(SkaEntity entity, SkaComponentIndex index, void* component);
void ska_ecs_component_manager_remove_component(SkaEntity entity, SkaComponentIndex index);
void ska_ecs_component_manager_remove_all_components(SkaEntity entity);
bool ska_ecs_component_manager_has_component(SkaEntity entity, SkaComponentIndex index);
void ska_ecs_component_manager_set_component_signature(SkaEntity entity, SkaComponentType componentTypeSignature);
SkaComponentType ska_ecs_component_manager_get_component_signature(SkaEntity entity);

const char* ska_ecs_component_get_component_data_index_string(SkaComponentIndex index);

#ifdef __cplusplus
}
#endif
