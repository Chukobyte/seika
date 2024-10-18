#pragma once

#include <stddef.h>

#include "seika/defines.h"

// Entity is defined as a unsigned 32-bit integer

// Gives a chance for users to set own max entities
#ifndef SKA_MAX_ENTITIES
#define SKA_MAX_ENTITIES 200000 // starting with 200,000 for now
#endif

#define SKA_NULL_ENTITY UINT32_MAX

typedef uint32 SkaEntity;

void ska_ecs_entity_initialize();
void ska_ecs_entity_finalize();
// Pops entity from the queue
SkaEntity ska_ecs_entity_create();
// Push entity to the queue
void ska_ecs_entity_return(SkaEntity entity);
usize ska_ecs_entity_get_active_count();
