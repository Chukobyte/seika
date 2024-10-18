#pragma once

#if SKA_ECS

#include "seika/defines.h"

// Entity is defined as a unsigned 32-bit integer
typedef uint32 SkaEntity;
#define SKA_NULL_ENTITY (SkaEntity)-1

void ska_ecs_entity_initialize();
void ska_ecs_entity_finalize();
// Pops entity from the queue
SkaEntity ska_ecs_entity_create();
// Push entity to the queue
void ska_ecs_entity_return(SkaEntity entity);
usize ska_ecs_entity_get_active_count();

#endif // if SKA_ECS
