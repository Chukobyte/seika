#include "ecs.h"

void ska_ecs_initialize() {
    ska_ecs_entity_initialize();
    ska_ecs_system_initialize();
    ska_ecs_component_manager_initialize();
}

void ska_ecs_finalize() {
    ska_ecs_entity_finalize();
    ska_ecs_component_manager_finalize();
    ska_ecs_system_finalize();
}
