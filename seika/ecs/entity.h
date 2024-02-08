#pragma once

#include <stdint.h>

// Entity is defined as a unsigned 32-bit integer

#define SKA_MAX_ENTITIES 200000 // starting with 200,000 for now
#define SKA_NULL_ENTITY UINT32_MAX

typedef uint32_t SkaEntity;

void ska_ecs_entity_initialize();
void ska_ecs_entity_finalize();
// Pops entity from the queue
SkaEntity ska_ecs_entity_create();
// Push entity to the queue
void ska_ecs_entity_return(SkaEntity entity);
