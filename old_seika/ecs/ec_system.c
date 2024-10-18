#include "ec_system.h"

#include <string.h>

#include "seika/string.h"
#include "seika/flag_utils.h"
#include "seika/logger.h"
#include "seika/memory.h"
#include "seika/assert.h"

//--- EC System Manager ---//
#define MAX_ENTITY_SYSTEMS_PER_HOOK 12

typedef struct EntitySystemData {
    usize entity_systems_count;
    usize on_entity_start_systems_count;
    usize on_entity_end_systems_count;
    usize on_entity_entered_scene_systems_count;
    usize render_systems_count;
    usize pre_update_all_systems_count;
    usize post_update_all_systems_count;
    usize update_systems_count;
    usize fixed_update_systems_count;
    usize network_callback_systems_count;
    SkaECSSystem* entity_systems[SKA_ECS_MAX_COMPONENTS];
    SkaECSSystem* on_entity_start_systems[MAX_ENTITY_SYSTEMS_PER_HOOK];
    SkaECSSystem* on_entity_end_systems[MAX_ENTITY_SYSTEMS_PER_HOOK];
    SkaECSSystem* on_entity_entered_scene_systems[MAX_ENTITY_SYSTEMS_PER_HOOK];
    SkaECSSystem* render_systems[MAX_ENTITY_SYSTEMS_PER_HOOK];
    SkaECSSystem* update_systems[MAX_ENTITY_SYSTEMS_PER_HOOK];
    SkaECSSystem* pre_update_all_systems[MAX_ENTITY_SYSTEMS_PER_HOOK];
    SkaECSSystem* post_update_all_systems[MAX_ENTITY_SYSTEMS_PER_HOOK];
    SkaECSSystem* fixed_update_systems[MAX_ENTITY_SYSTEMS_PER_HOOK];
    SkaECSSystem* network_callback_systems[MAX_ENTITY_SYSTEMS_PER_HOOK];
} EntitySystemData;

void ska_ecs_system_insert_entity_into_system(SkaEntity entity, SkaECSSystem* system);
void ska_ecs_system_remove_entity_from_system(SkaEntity entity, SkaECSSystem* system);

static EntitySystemData entitySystemData;

void ska_ecs_system_initialize() {
    // Initialize system data to 0
    entitySystemData = (EntitySystemData){0};
}

void ska_ecs_system_finalize() {
    for (usize i = 0; i < entitySystemData.entity_systems_count; i++) {
        ska_ecs_system_destroy(entitySystemData.entity_systems[i]);
        entitySystemData.entity_systems[i] = NULL;
    }
}

SkaECSSystem* ska_ecs_system_create(const char* systemName) {
    SkaECSSystem* newSystem = SKA_MEM_ALLOCATE(SkaECSSystem);
    newSystem->name = ska_strdup(systemName);
    return newSystem;
}

SkaECSSystem* ska_ecs_system_create_from_template(SkaECSSystemTemplate* systemTemplate) {
    SkaECSSystem* newSystem = SKA_MEM_ALLOCATE(SkaECSSystem);
    newSystem->name = ska_strdup(systemTemplate->name);
    newSystem->on_ec_system_register = systemTemplate->on_ec_system_register;
    newSystem->on_ec_system_destroy = systemTemplate->on_ec_system_destroy;
    newSystem->on_entity_registered_func = systemTemplate->on_entity_registered_func;
    newSystem->on_entity_start_func = systemTemplate->on_entity_start_func;
    newSystem->on_entity_end_func = systemTemplate->on_entity_end_func;
    newSystem->on_entity_unregistered_func = systemTemplate->on_entity_unregistered_func;
    newSystem->on_entity_entered_scene_func = systemTemplate->on_entity_entered_scene_func;
    newSystem->render_func = systemTemplate->render_func;
    newSystem->pre_update_all_func = systemTemplate->pre_update_all_func;
    newSystem->post_update_all_func = systemTemplate->post_update_all_func;
    newSystem->update_func = systemTemplate->update_func;
    newSystem->fixed_update_func = systemTemplate->fixed_update_func;
    newSystem->network_callback_func = systemTemplate->network_callback_func;
    return newSystem;
}

SkaECSSystemTemplate ska_ecs_system_create_default_template(const char* systemName) {
    return (SkaECSSystemTemplate){
        .name = systemName, .on_ec_system_register = NULL, .on_ec_system_destroy = NULL, .on_entity_registered_func = NULL, .on_entity_start_func = NULL, .on_entity_end_func = NULL,
        .on_entity_unregistered_func = NULL, .on_entity_entered_scene_func = NULL, .render_func = NULL, .pre_update_all_func = NULL, .post_update_all_func = NULL,
        .update_func = NULL, .fixed_update_func = NULL, .network_callback_func = NULL
    };
}

static SkaECSSystem* update_system_with_type_signature_string(SkaECSSystem* system, const char* signatures) {
    // Take signature string, create flags, and add to system's component signature
    static char typeNameBuffer[256];
    char* word = typeNameBuffer;
    char* start = word;
    for (const char* src = signatures; *src != '\0'; ++src) {
        // Skip commas and whitespace
        if (*src == ',') {
            *word = '\0';
            const SkaComponentTypeInfo* typeInfo = ska_ecs_component_find_type_info(typeNameBuffer);
            SKA_ASSERT_FMT(typeInfo, "Unable to get type info for '%s'", typeNameBuffer);
            system->component_signature |= typeInfo->type;
            word = start;
            continue;
        } else if (*src == ' ') {
            continue;
        }
        *word++ = *src;
    }

    *word = '\0';
    const SkaComponentTypeInfo* typeInfo = ska_ecs_component_find_type_info(typeNameBuffer);
    SKA_ASSERT_FMT(typeInfo, "Unable to get type info for '%s'", typeNameBuffer);
    system->component_signature |= typeInfo->type;
    return system;
}

SkaECSSystem* ska_ecs_system_create_with_signature_string(const char* systemName, const char* signatures) {
    SkaECSSystem* newSystem = ska_ecs_system_create(systemName);
    update_system_with_type_signature_string(newSystem, signatures);
    return newSystem;
}

SkaECSSystem* ska_ecs_system_create_from_template_with_signature_string(SkaECSSystemTemplate* systemTemplate, const char* signatures) {
    SkaECSSystem* newSystem = ska_ecs_system_create_from_template(systemTemplate);
    update_system_with_type_signature_string(newSystem, signatures);
    return newSystem;
}

void ska_ecs_system_destroy(SkaECSSystem* entitySystem) {
    if (entitySystem->on_ec_system_destroy) {
        entitySystem->on_ec_system_destroy(entitySystem);
    }
    SKA_MEM_FREE(entitySystem);
}

void ska_ecs_system_register(SkaECSSystem* system) {
    SKA_ASSERT_FMT(system != NULL, "Passed in system is NULL!");
    SKA_ASSERT_FMT(entitySystemData.entity_systems_count + 1 < SKA_ECS_MAX_COMPONENTS, "At system limit of '%d'", SKA_ECS_MAX_COMPONENTS);
    entitySystemData.entity_systems[entitySystemData.entity_systems_count++] = system;
    if (system->on_ec_system_register != NULL) {
        system->on_ec_system_register(system);
    }
    if (system->on_entity_start_func != NULL) {
        SKA_ASSERT_FMT(entitySystemData.on_entity_start_systems_count + 1 < MAX_ENTITY_SYSTEMS_PER_HOOK, "At system 'on_entity_start_func' limit of '%d'", MAX_ENTITY_SYSTEMS_PER_HOOK);
        entitySystemData.on_entity_start_systems[entitySystemData.on_entity_start_systems_count++] = system;
    }
    if (system->on_entity_end_func != NULL) {
        SKA_ASSERT_FMT(entitySystemData.on_entity_end_systems_count + 1 < MAX_ENTITY_SYSTEMS_PER_HOOK, "At system 'on_entity_end_func' limit of '%d'", MAX_ENTITY_SYSTEMS_PER_HOOK);
        entitySystemData.on_entity_end_systems[entitySystemData.on_entity_end_systems_count++] = system;
    }
    if (system->on_entity_entered_scene_func != NULL) {
        SKA_ASSERT_FMT(entitySystemData.on_entity_entered_scene_systems_count + 1 < MAX_ENTITY_SYSTEMS_PER_HOOK, "At system 'on_entity_entered_scene_func' limit of '%d'", MAX_ENTITY_SYSTEMS_PER_HOOK);
        entitySystemData.on_entity_entered_scene_systems[entitySystemData.on_entity_entered_scene_systems_count++] = system;
    }
    if (system->render_func != NULL) {
        SKA_ASSERT_FMT(entitySystemData.render_systems_count + 1 < MAX_ENTITY_SYSTEMS_PER_HOOK, "At system 'render_func' limit of '%d'", MAX_ENTITY_SYSTEMS_PER_HOOK);
        entitySystemData.render_systems[entitySystemData.render_systems_count++] = system;
    }
    if (system->pre_update_all_func != NULL) {
        SKA_ASSERT_FMT(entitySystemData.pre_update_all_systems_count + 1 < MAX_ENTITY_SYSTEMS_PER_HOOK, "At system 'pre_update_all_func' limit of '%d'", MAX_ENTITY_SYSTEMS_PER_HOOK);
        entitySystemData.pre_update_all_systems[entitySystemData.pre_update_all_systems_count++] = system;
    }
    if (system->post_update_all_func != NULL) {
        SKA_ASSERT_FMT(entitySystemData.post_update_all_systems_count + 1 < MAX_ENTITY_SYSTEMS_PER_HOOK, "At system 'post_update_all_func' limit of '%d'", MAX_ENTITY_SYSTEMS_PER_HOOK);
        entitySystemData.post_update_all_systems[entitySystemData.post_update_all_systems_count++] = system;
    }
    if (system->update_func != NULL) {
        SKA_ASSERT_FMT(entitySystemData.update_systems_count + 1 < MAX_ENTITY_SYSTEMS_PER_HOOK, "At system 'update_func' limit of '%d'", MAX_ENTITY_SYSTEMS_PER_HOOK);
        entitySystemData.update_systems[entitySystemData.update_systems_count++] = system;
    }
    if (system->fixed_update_func != NULL) {
        SKA_ASSERT_FMT(entitySystemData.fixed_update_systems_count + 1 < MAX_ENTITY_SYSTEMS_PER_HOOK, "At system 'fixed_update_func' limit of '%d'", MAX_ENTITY_SYSTEMS_PER_HOOK);
        entitySystemData.fixed_update_systems[entitySystemData.fixed_update_systems_count++] = system;
    }
    if (system->network_callback_func != NULL) {
        SKA_ASSERT_FMT(entitySystemData.network_callback_systems_count + 1 < MAX_ENTITY_SYSTEMS_PER_HOOK, "At system 'network_callback_func' limit of '%d'", MAX_ENTITY_SYSTEMS_PER_HOOK);
        entitySystemData.network_callback_systems[entitySystemData.network_callback_systems_count++] = system;
    }
}

void ska_ecs_system_update_entity_signature_with_systems(SkaEntity entity) {
    const SkaComponentType entityComponentSignature = ska_ecs_component_manager_get_component_signature(entity);
    for (usize i = 0; i < entitySystemData.entity_systems_count; i++) {
        if (SKA_FLAG_CONTAINS(entityComponentSignature, entitySystemData.entity_systems[i]->component_signature)) {
            ska_ecs_system_insert_entity_into_system(entity, entitySystemData.entity_systems[i]);
        } else {
            ska_ecs_system_remove_entity_from_system(entity, entitySystemData.entity_systems[i]);
        }
    }
}

void ska_ecs_system_event_entity_start(SkaEntity entity) {
    const SkaComponentType entityComponentSignature = ska_ecs_component_manager_get_component_signature(entity);
    for (usize i = 0; i < entitySystemData.on_entity_start_systems_count; i++) {
        SkaECSSystem* ecsSystem = entitySystemData.on_entity_start_systems[i];
        if (SKA_FLAG_CONTAINS(entityComponentSignature, ecsSystem->component_signature)) {
            ecsSystem->on_entity_start_func(ecsSystem, entity);
        }
    }
}

void ska_ecs_system_event_entity_end(SkaEntity entity) {
    // Notify scene exit observers before calling it on systems
    // TODO: Consider hooks for components instead of direct node component references
    const SkaComponentType entityComponentSignature = ska_ecs_component_manager_get_component_signature(entity);
    for (usize i = 0; i < entitySystemData.on_entity_end_systems_count; i++) {
        SkaECSSystem* ecsSystem = entitySystemData.on_entity_end_systems[i];
        if (SKA_FLAG_CONTAINS(entityComponentSignature, ecsSystem->component_signature)) {
            ecsSystem->on_entity_end_func(ecsSystem, entity);
        }
    }
//    NodeComponent* nodeComponent = (NodeComponent*)ska_ecs_component_manager_get_component_unchecked(entity, CreComponentDataIndex_NODE);
//    if (nodeComponent != NULL) {
//        // Note: Node events should not be created during this time
//        ska_event_notify_observers(&nodeComponent->onSceneTreeExit, &(SkaSubjectNotifyPayload) {
//                .data = &entity
//        });
//    }
}

void ska_ecs_system_event_entity_entered_scene(SkaEntity entity) {
    // Notify scene enter observers before calling it on systems
//    NodeComponent* nodeComponent = (NodeComponent*) ska_ecs_component_manager_get_component_unchecked(entity, CreComponentDataIndex_NODE);
//    if (nodeComponent != NULL) {
//        ska_event_notify_observers(&nodeComponent->onSceneTreeEnter, &(SkaSubjectNotifyPayload) {
//                .data = &entity
//        });
//    }
    const SkaComponentType entityComponentSignature = ska_ecs_component_manager_get_component_signature(entity);
    for (usize i = 0; i < entitySystemData.on_entity_entered_scene_systems_count; i++) {
        SkaECSSystem* ecsSystem = entitySystemData.on_entity_entered_scene_systems[i];
        if (SKA_FLAG_CONTAINS(entityComponentSignature, ecsSystem->component_signature)) {
            ecsSystem->on_entity_entered_scene_func(ecsSystem, entity);
        }
    }
}

void ska_ecs_system_event_render_systems() {
    for (usize i = 0; i < entitySystemData.render_systems_count; i++) {
        SkaECSSystem* ecsSystem = entitySystemData.render_systems[i];
        ecsSystem->render_func(ecsSystem);
    }
}

void ska_ecs_system_event_pre_update_all_systems() {
    for (usize i = 0; i < entitySystemData.pre_update_all_systems_count; i++) {
        SkaECSSystem* ecsSystem = entitySystemData.pre_update_all_systems[i];
        ecsSystem->pre_update_all_func(ecsSystem);
    }
}

void ska_ecs_system_event_post_update_all_systems() {
    for (usize i = 0; i < entitySystemData.post_update_all_systems_count; i++) {
        SkaECSSystem* ecsSystem = entitySystemData.post_update_all_systems[i];
        ecsSystem->post_update_all_func(ecsSystem);
    }
}

void ska_ecs_system_event_update_systems(f32 deltaTime) {
    for (usize i = 0; i < entitySystemData.update_systems_count; i++) {
        SkaECSSystem* ecsSystem = entitySystemData.update_systems[i];
        ecsSystem->update_func(ecsSystem, deltaTime);
    }
}

void ska_ecs_system_event_fixed_update_systems(f32 deltaTime) {
    for (usize i = 0; i < entitySystemData.fixed_update_systems_count; i++) {
        SkaECSSystem* ecsSystem = entitySystemData.fixed_update_systems[i];
        ecsSystem->fixed_update_func(ecsSystem, deltaTime);
    }
}

void ska_ecs_system_event_network_callback(const char* message) {
    for (usize i = 0; i < entitySystemData.network_callback_systems_count; i++) {
        SkaECSSystem* ecsSystem = entitySystemData.network_callback_systems[i];
        ecsSystem->network_callback_func(ecsSystem, message);
    }
}

// --- Internal Functions --- //
bool ska_ecs_system_has_entity(SkaEntity entity, SkaECSSystem* system) {
    for (usize i = 0; i < system->entity_count; i++) {
        if (entity == system->entities[i]) {
            return true;
        }
    }
    return false;
}

void ska_ecs_system_insert_entity_into_system(SkaEntity entity, SkaECSSystem* system) {
    if (!ska_ecs_system_has_entity(entity, system)) {
        system->entities[system->entity_count++] = entity;
        if (system->on_entity_registered_func != NULL) {
            system->on_entity_registered_func(system, entity);
        }
    } else {
        ska_logger_internal_queue_message("Entity '%d' already in system '%s'", entity, system->name);
    }
}

void ska_ecs_system_remove_entity_from_system(SkaEntity entity, SkaECSSystem* system) {
    for (usize i = 0; i < system->entity_count; i++) {
        if (entity == system->entities[i]) {
            // Entity found
            if (i + 1 < SKA_MAX_ENTITIES) {
                system->entities[i] = SKA_NULL_ENTITY;
            }
            if (system->on_entity_unregistered_func != NULL) {
                system->on_entity_unregistered_func(system, entity);
            }

            // Condense array
            for (usize newIndex = i; i < system->entity_count; i++) {
                if (system->entities[i] == SKA_NULL_ENTITY) {
                    // Early exit if 2 consecutive nulls
                    if (system->entities[i + 1] == SKA_NULL_ENTITY) {
                        break;
                    }
                    system->entities[i] = system->entities[i + 1];
                    system->entities[i + 1] = SKA_NULL_ENTITY;
                }
            }

            system->entity_count--;
            break;
        }
    }
}

void ska_ecs_system_remove_entity_from_all_systems(SkaEntity entity) {
    for (usize i = 0; i < entitySystemData.entity_systems_count; i++) {
        ska_ecs_system_remove_entity_from_system(entity, entitySystemData.entity_systems[i]);
    }
}
