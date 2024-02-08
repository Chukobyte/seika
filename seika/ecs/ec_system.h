#pragma once

#include "component.h"

#define SKA_ECS_SYSTEM_CREATE(Name, ...) \
ska_ecs_system_create_with_signature_string(Name, #__VA_ARGS__)

typedef void (*OnEntityRegisteredFunc) (SkaEntity); // init
typedef void (*OnEntityStartFunc) (SkaEntity); // Useful for after initialization functionality (such as entering a scene)
typedef void (*OnEntityEndFunc) (SkaEntity); // Useful for before deletion functionality (such as leaving a scene)
typedef void (*OnEntityUnRegisteredFunc) (SkaEntity); // delete
typedef void (*OnEntityEnteredSceneFunc) (SkaEntity); // When entity enters a scene (after _start() is called)
typedef void (*RenderFunc) ();
typedef void (*PreProcessAllFunc) ();
typedef void (*PostProcessAllFunc) ();
typedef void (*UpdateFunc) (float);
typedef void (*FixedUpdateFunc) (float);
typedef void (*NetworkCallbackFunc) (const char*);
typedef void (*OnECSystemDestroy) ();

typedef struct SkaECSSystem {
    char* name;
    // Event callbacks
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
    OnECSystemDestroy on_ec_system_destroy;
    SkaComponentType component_signature;
    size_t entity_count;
    // Keeps track of entities that match a component signature
    SkaEntity entities[SKA_MAX_ENTITIES];
} SkaECSSystem;

void ska_ecs_system_initialize();
void ska_ecs_system_finalize();
SkaECSSystem* ska_ecs_system_create(const char* systemName);
SkaECSSystem* ska_ecs_system_create_with_signature_string(const char* systemName, const char* signatures);
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
void ska_ecs_system_event_update_systems(float deltaTime);
void ska_ecs_system_event_fixed_update_systems(float deltaTime);
void ska_ecs_system_event_network_callback(const char* message);
