#if SKA_ECS

#include "ec_system.h"

#include "seika/string.h"
#include "seika/flag_utils.h"
#include "../logger.h"
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

static EntitySystemData entity_system_data;

void ska_ecs_system_initialize() {
    // Initialize system data to 0
    entity_system_data = (EntitySystemData){0};
}

void ska_ecs_system_finalize() {
    for (usize i = 0; i < entity_system_data.entity_systems_count; i++) {
        ska_ecs_system_destroy(entity_system_data.entity_systems[i]);
        entity_system_data.entity_systems[i] = NULL;
    }
}

SkaECSSystem* ska_ecs_system_create(const char* system_name) {
    SkaECSSystem* new_system = SKA_ALLOC_ZEROED(SkaECSSystem);
    new_system->name = ska_strdup(system_name);
    new_system->entities = ska_array_list_init_default_capacity(sizeof(SkaEntity));
    return new_system;
}

SkaECSSystem* ska_ecs_system_create_from_template(SkaECSSystemTemplate* system_template) {
    SkaECSSystem* new_system = ska_ecs_system_create(system_template->name);
    new_system->on_ec_system_register = system_template->on_ec_system_register;
    new_system->on_ec_system_destroy = system_template->on_ec_system_destroy;
    new_system->on_entity_registered_func = system_template->on_entity_registered_func;
    new_system->on_entity_start_func = system_template->on_entity_start_func;
    new_system->on_entity_end_func = system_template->on_entity_end_func;
    new_system->on_entity_unregistered_func = system_template->on_entity_unregistered_func;
    new_system->on_entity_entered_scene_func = system_template->on_entity_entered_scene_func;
    new_system->render_func = system_template->render_func;
    new_system->pre_update_all_func = system_template->pre_update_all_func;
    new_system->post_update_all_func = system_template->post_update_all_func;
    new_system->update_func = system_template->update_func;
    new_system->fixed_update_func = system_template->fixed_update_func;
    new_system->network_callback_func = system_template->network_callback_func;
    return new_system;
}

SkaECSSystemTemplate ska_ecs_system_create_default_template(const char* system_name) {
    return (SkaECSSystemTemplate){
        .name = system_name, .on_ec_system_register = NULL, .on_ec_system_destroy = NULL, .on_entity_registered_func = NULL, .on_entity_start_func = NULL, .on_entity_end_func = NULL,
        .on_entity_unregistered_func = NULL, .on_entity_entered_scene_func = NULL, .render_func = NULL, .pre_update_all_func = NULL, .post_update_all_func = NULL,
        .update_func = NULL, .fixed_update_func = NULL, .network_callback_func = NULL
    };
}

static SkaECSSystem* update_system_with_type_signature_string(SkaECSSystem* system, const char* signatures) {
    // Take signature string, create flags, and add to system's component signature
    static char type_name_buffer[256];
    char* word = type_name_buffer;
    char* start = word;
    for (const char* src = signatures; *src != '\0'; ++src) {
        // Skip commas and whitespace
        if (*src == ',') {
            *word = '\0';
            const SkaComponentTypeInfo* type_info = ska_ecs_component_find_type_info(type_name_buffer);
            SKA_ASSERT_FMT(type_info, "Unable to get type info for '%s'", type_name_buffer);
            system->component_signature |= type_info->type;
            word = start;
            continue;
        } else if (*src == ' ') {
            continue;
        }
        *word++ = *src;
    }

    *word = '\0';
    const SkaComponentTypeInfo* type_info = ska_ecs_component_find_type_info(type_name_buffer);
    SKA_ASSERT_FMT(type_info, "Unable to get type info for '%s'", type_name_buffer);
    system->component_signature |= type_info->type;
    return system;
}

SkaECSSystem* ska_ecs_system_create_with_signature_string(const char* system_name, const char* signatures) {
    SkaECSSystem* new_system = ska_ecs_system_create(system_name);
    update_system_with_type_signature_string(new_system, signatures);
    return new_system;
}

SkaECSSystem* ska_ecs_system_create_from_template_with_signature_string(SkaECSSystemTemplate* system_template, const char* signatures) {
    SkaECSSystem* new_system = ska_ecs_system_create_from_template(system_template);
    update_system_with_type_signature_string(new_system, signatures);
    return new_system;
}

void ska_ecs_system_destroy(SkaECSSystem* entity_system) {
    if (entity_system->on_ec_system_destroy) {
        entity_system->on_ec_system_destroy(entity_system);
    }
    ska_array_list_deinit(&entity_system->entities);
    SKA_FREE(entity_system->name);
    SKA_FREE(entity_system);
}

void ska_ecs_system_register(SkaECSSystem* system) {
    SKA_ASSERT_FMT(system != NULL, "Passed in system is NULL!");
    SKA_ASSERT_FMT(entity_system_data.entity_systems_count + 1 < SKA_ECS_MAX_COMPONENTS, "At system limit of '%d'", SKA_ECS_MAX_COMPONENTS);
    entity_system_data.entity_systems[entity_system_data.entity_systems_count++] = system;
    if (system->on_ec_system_register != NULL) {
        system->on_ec_system_register(system);
    }
    if (system->on_entity_start_func != NULL) {
        SKA_ASSERT_FMT(entity_system_data.on_entity_start_systems_count + 1 < MAX_ENTITY_SYSTEMS_PER_HOOK, "At system 'on_entity_start_func' limit of '%d'", MAX_ENTITY_SYSTEMS_PER_HOOK);
        entity_system_data.on_entity_start_systems[entity_system_data.on_entity_start_systems_count++] = system;
    }
    if (system->on_entity_end_func != NULL) {
        SKA_ASSERT_FMT(entity_system_data.on_entity_end_systems_count + 1 < MAX_ENTITY_SYSTEMS_PER_HOOK, "At system 'on_entity_end_func' limit of '%d'", MAX_ENTITY_SYSTEMS_PER_HOOK);
        entity_system_data.on_entity_end_systems[entity_system_data.on_entity_end_systems_count++] = system;
    }
    if (system->on_entity_entered_scene_func != NULL) {
        SKA_ASSERT_FMT(entity_system_data.on_entity_entered_scene_systems_count + 1 < MAX_ENTITY_SYSTEMS_PER_HOOK, "At system 'on_entity_entered_scene_func' limit of '%d'", MAX_ENTITY_SYSTEMS_PER_HOOK);
        entity_system_data.on_entity_entered_scene_systems[entity_system_data.on_entity_entered_scene_systems_count++] = system;
    }
    if (system->render_func != NULL) {
        SKA_ASSERT_FMT(entity_system_data.render_systems_count + 1 < MAX_ENTITY_SYSTEMS_PER_HOOK, "At system 'render_func' limit of '%d'", MAX_ENTITY_SYSTEMS_PER_HOOK);
        entity_system_data.render_systems[entity_system_data.render_systems_count++] = system;
    }
    if (system->pre_update_all_func != NULL) {
        SKA_ASSERT_FMT(entity_system_data.pre_update_all_systems_count + 1 < MAX_ENTITY_SYSTEMS_PER_HOOK, "At system 'pre_update_all_func' limit of '%d'", MAX_ENTITY_SYSTEMS_PER_HOOK);
        entity_system_data.pre_update_all_systems[entity_system_data.pre_update_all_systems_count++] = system;
    }
    if (system->post_update_all_func != NULL) {
        SKA_ASSERT_FMT(entity_system_data.post_update_all_systems_count + 1 < MAX_ENTITY_SYSTEMS_PER_HOOK, "At system 'post_update_all_func' limit of '%d'", MAX_ENTITY_SYSTEMS_PER_HOOK);
        entity_system_data.post_update_all_systems[entity_system_data.post_update_all_systems_count++] = system;
    }
    if (system->update_func != NULL) {
        SKA_ASSERT_FMT(entity_system_data.update_systems_count + 1 < MAX_ENTITY_SYSTEMS_PER_HOOK, "At system 'update_func' limit of '%d'", MAX_ENTITY_SYSTEMS_PER_HOOK);
        entity_system_data.update_systems[entity_system_data.update_systems_count++] = system;
    }
    if (system->fixed_update_func != NULL) {
        SKA_ASSERT_FMT(entity_system_data.fixed_update_systems_count + 1 < MAX_ENTITY_SYSTEMS_PER_HOOK, "At system 'fixed_update_func' limit of '%d'", MAX_ENTITY_SYSTEMS_PER_HOOK);
        entity_system_data.fixed_update_systems[entity_system_data.fixed_update_systems_count++] = system;
    }
    if (system->network_callback_func != NULL) {
        SKA_ASSERT_FMT(entity_system_data.network_callback_systems_count + 1 < MAX_ENTITY_SYSTEMS_PER_HOOK, "At system 'network_callback_func' limit of '%d'", MAX_ENTITY_SYSTEMS_PER_HOOK);
        entity_system_data.network_callback_systems[entity_system_data.network_callback_systems_count++] = system;
    }
}

void ska_ecs_system_update_entity_signature_with_systems(SkaEntity entity) {
    const SkaComponentType entity_component_signature = ska_ecs_component_manager_get_component_signature(entity);
    for (usize i = 0; i < entity_system_data.entity_systems_count; i++) {
        if (SKA_FLAG_CONTAINS(entity_component_signature, entity_system_data.entity_systems[i]->component_signature)) {
            ska_ecs_system_insert_entity_into_system(entity, entity_system_data.entity_systems[i]);
        } else {
            ska_ecs_system_remove_entity_from_system(entity, entity_system_data.entity_systems[i]);
        }
    }
}

void ska_ecs_system_event_entity_start(SkaEntity entity) {
    const SkaComponentType entity_component_signature = ska_ecs_component_manager_get_component_signature(entity);
    for (usize i = 0; i < entity_system_data.on_entity_start_systems_count; i++) {
        SkaECSSystem* ecs_system = entity_system_data.on_entity_start_systems[i];
        if (SKA_FLAG_CONTAINS(entity_component_signature, ecs_system->component_signature)) {
            ecs_system->on_entity_start_func(ecs_system, entity);
        }
    }
}

void ska_ecs_system_event_entity_end(SkaEntity entity) {
    // Notify scene exit observers before calling it on systems
    // TODO: Consider hooks for components instead of direct node component references
    const SkaComponentType entity_component_signature = ska_ecs_component_manager_get_component_signature(entity);
    for (usize i = 0; i < entity_system_data.on_entity_end_systems_count; i++) {
        SkaECSSystem* ecs_system = entity_system_data.on_entity_end_systems[i];
        if (SKA_FLAG_CONTAINS(entity_component_signature, ecs_system->component_signature)) {
            ecs_system->on_entity_end_func(ecs_system, entity);
        }
    }
}

void ska_ecs_system_event_entity_entered_scene(SkaEntity entity) {
    const SkaComponentType entity_component_signature = ska_ecs_component_manager_get_component_signature(entity);
    for (usize i = 0; i < entity_system_data.on_entity_entered_scene_systems_count; i++) {
        SkaECSSystem* ecs_system = entity_system_data.on_entity_entered_scene_systems[i];
        if (SKA_FLAG_CONTAINS(entity_component_signature, ecs_system->component_signature)) {
            ecs_system->on_entity_entered_scene_func(ecs_system, entity);
        }
    }
}

void ska_ecs_system_event_render_systems() {
    for (usize i = 0; i < entity_system_data.render_systems_count; i++) {
        SkaECSSystem* ecs_system = entity_system_data.render_systems[i];
        ecs_system->render_func(ecs_system);
    }
}

void ska_ecs_system_event_pre_update_all_systems() {
    for (usize i = 0; i < entity_system_data.pre_update_all_systems_count; i++) {
        SkaECSSystem* ecs_system = entity_system_data.pre_update_all_systems[i];
        ecs_system->pre_update_all_func(ecs_system);
    }
}

void ska_ecs_system_event_post_update_all_systems() {
    for (usize i = 0; i < entity_system_data.post_update_all_systems_count; i++) {
        SkaECSSystem* ecs_system = entity_system_data.post_update_all_systems[i];
        ecs_system->post_update_all_func(ecs_system);
    }
}

void ska_ecs_system_event_update_systems(f32 delta_time) {
    for (usize i = 0; i < entity_system_data.update_systems_count; i++) {
        SkaECSSystem* ecs_system = entity_system_data.update_systems[i];
        ecs_system->update_func(ecs_system, delta_time);
    }
}

void ska_ecs_system_event_fixed_update_systems(f32 delta_time) {
    for (usize i = 0; i < entity_system_data.fixed_update_systems_count; i++) {
        SkaECSSystem* ecs_system = entity_system_data.fixed_update_systems[i];
        ecs_system->fixed_update_func(ecs_system, delta_time);
    }
}

void ska_ecs_system_event_network_callback(const char* message) {
    for (usize i = 0; i < entity_system_data.network_callback_systems_count; i++) {
        SkaECSSystem* ecs_system = entity_system_data.network_callback_systems[i];
        ecs_system->network_callback_func(ecs_system, message);
    }
}

bool ska_ecs_system_has_entity(SkaEntity entity, SkaECSSystem* system) {
    return ska_array_list_has(&system->entities, &entity);
}

void ska_ecs_system_remove_entity_from_all_systems(SkaEntity entity) {
    for (usize i = 0; i < entity_system_data.entity_systems_count; i++) {
        ska_ecs_system_remove_entity_from_system(entity, entity_system_data.entity_systems[i]);
    }
}

// --- Internal Functions --- //

void ska_ecs_system_insert_entity_into_system(SkaEntity entity, SkaECSSystem* system) {
    if (!ska_ecs_system_has_entity(entity, system)) {
        ska_array_list_append(&system->entities, &entity);
        if (system->on_entity_registered_func != NULL) {
            system->on_entity_registered_func(system, entity);
        }
    } else {
        ska_logger_internal_queue_message("Entity '%d' already in system '%s'", entity, system->name);
    }
}

void ska_ecs_system_remove_entity_from_system(SkaEntity entity, SkaECSSystem* system) {
    ska_array_list_remove(&system->entities, &entity);
}

#endif // if SKA_ECS
