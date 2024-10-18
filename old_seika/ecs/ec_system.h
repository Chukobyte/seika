#pragma once

#include "component.h"

#define SKA_ECS_SYSTEM_CREATE(NAME, ...) \
ska_ecs_system_create_with_signature_string(NAME, #__VA_ARGS__)

#define SKA_ECS_SYSTEM_CREATE_FROM_TEMPLATE(TEMPLATE, ...) \
ska_ecs_system_create_from_template_with_signature_string(TEMPLATE, #__VA_ARGS__)

// Creates and register system from template
#define SKA_ECS_SYSTEM_REGISTER_FROM_TEMPLATE(TEMPLATE, ...) \
ska_ecs_system_register(ska_ecs_system_create_from_template_with_signature_string(TEMPLATE, #__VA_ARGS__))

struct SkaECSSystem;

typedef void (*OnECSystemRegister) (struct SkaECSSystem*);
typedef void (*OnECSystemDestroy) (struct SkaECSSystem*);
typedef void (*OnEntityRegisteredFunc) (struct SkaECSSystem*, SkaEntity); // init
typedef void (*OnEntityStartFunc) (struct SkaECSSystem*, SkaEntity); // Useful for after initialization functionality (such as entering a scene)
typedef void (*OnEntityEndFunc) (struct SkaECSSystem*, SkaEntity); // Useful for before deletion functionality (such as leaving a scene)
typedef void (*OnEntityUnRegisteredFunc) (struct SkaECSSystem*, SkaEntity); // delete
typedef void (*OnEntityEnteredSceneFunc) (struct SkaECSSystem*, SkaEntity); // When entity enters a scene (after _start() is called)
typedef void (*RenderFunc) (struct SkaECSSystem*);
typedef void (*PreProcessAllFunc) (struct SkaECSSystem*);
typedef void (*PostProcessAllFunc) (struct SkaECSSystem*);
typedef void (*UpdateFunc) (struct SkaECSSystem*, float);
typedef void (*FixedUpdateFunc) (struct SkaECSSystem*, float);
typedef void (*NetworkCallbackFunc) (struct SkaECSSystem*, const char*);

typedef struct SkaECSSystem {
    char* name;
    // Event callbacks
    OnECSystemRegister on_ec_system_register; // happens on register
    OnECSystemDestroy on_ec_system_destroy;
    OnEntityRegisteredFunc on_entity_registered_func;
    OnEntityStartFunc on_entity_start_func;
    OnEntityEndFunc on_entity_end_func;
    OnEntityUnRegisteredFunc on_entity_unregistered_func;
    OnEntityEnteredSceneFunc on_entity_entered_scene_func;
    RenderFunc render_func;
    PreProcessAllFunc pre_update_all_func;
    PostProcessAllFunc post_update_all_func;
    UpdateFunc update_func;
    FixedUpdateFunc fixed_update_func;
    NetworkCallbackFunc network_callback_func;
    SkaComponentType component_signature;
    usize entity_count;
    // Keeps track of entities that match a component signature
    SkaEntity entities[SKA_MAX_ENTITIES];
} SkaECSSystem;

typedef struct SkaECSSystemTemplate {
    const char* name;
    OnECSystemRegister on_ec_system_register;
    OnECSystemDestroy on_ec_system_destroy;
    OnEntityRegisteredFunc on_entity_registered_func;
    OnEntityStartFunc on_entity_start_func;
    OnEntityEndFunc on_entity_end_func;
    OnEntityUnRegisteredFunc on_entity_unregistered_func;
    OnEntityEnteredSceneFunc on_entity_entered_scene_func;
    RenderFunc render_func;
    PreProcessAllFunc pre_update_all_func;
    PostProcessAllFunc post_update_all_func;
    UpdateFunc update_func;
    FixedUpdateFunc fixed_update_func;
    NetworkCallbackFunc network_callback_func;
} SkaECSSystemTemplate;

// example system template
#define SKA_ECS_SYSTEM_TEMPLATE_STRUCT SKA_STRUCT_LITERAL(SkaECSSystemTemplate){ \
    .name = "example template system", \
    .on_ec_system_register = NULL, \
    .on_ec_system_destroy = NULL, \
    .on_entity_registered_func = NULL, \
    .on_entity_start_func = NULL, \
    .on_entity_end_func = NULL, \
    .on_entity_unregistered_func = NULL, \
    .on_entity_entered_scene_func = NULL, \
    .render_func = NULL, \
    .pre_update_all_func = NULL, \
    .post_update_all_func = NULL, \
    .update_func = NULL, \
    .fixed_update_func = NULL, \
    .network_callback_func = NULL \
}

void ska_ecs_system_initialize();
void ska_ecs_system_finalize();
SkaECSSystem* ska_ecs_system_create(const char* systemName);
SkaECSSystem* ska_ecs_system_create_from_template(SkaECSSystemTemplate* systemTemplate);
SkaECSSystem* ska_ecs_system_create_with_signature_string(const char* systemName, const char* signatures);
SkaECSSystem* ska_ecs_system_create_from_template_with_signature_string(SkaECSSystemTemplate* systemTemplate, const char* signatures);
SkaECSSystemTemplate ska_ecs_system_create_default_template(const char* systemName);
void ska_ecs_system_destroy(SkaECSSystem* entitySystem);
void ska_ecs_system_register(SkaECSSystem* system);
void ska_ecs_system_update_entity_signature_with_systems(SkaEntity entity);
void ska_ecs_system_remove_entity_from_all_systems(SkaEntity entity);
bool ska_ecs_system_has_entity(SkaEntity entity, SkaECSSystem* system);

// Event functions
void ska_ecs_system_event_entity_start(SkaEntity entity);
void ska_ecs_system_event_entity_end(SkaEntity entity);
void ska_ecs_system_event_entity_entered_scene(SkaEntity entity);
void ska_ecs_system_event_render_systems();
void ska_ecs_system_event_pre_update_all_systems();
void ska_ecs_system_event_post_update_all_systems();
void ska_ecs_system_event_update_systems(f32 deltaTime);
void ska_ecs_system_event_fixed_update_systems(f32 deltaTime);
void ska_ecs_system_event_network_callback(const char* message);
