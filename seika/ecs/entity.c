#if SKA_ECS

#include "entity.h"
#include "seika/data_structures/id_queue.h"
#include "seika/assert.h"

#define SKA_INITIAL_ENTITY_CAPACITY 1000

static SkaIdQueue* entity_id_queue = NULL;
static usize active_entity_count = 0;

void ska_ecs_entity_initialize() {
    SKA_ASSERT(entity_id_queue == NULL);
    entity_id_queue = ska_id_queue_create(SKA_INITIAL_ENTITY_CAPACITY);
}

void ska_ecs_entity_finalize() {
    SKA_ASSERT(entity_id_queue);
    ska_id_queue_destroy(entity_id_queue);
    entity_id_queue = NULL;
    active_entity_count = 0;
}

SkaEntity ska_ecs_entity_create() {
    SKA_ASSERT(entity_id_queue);
    SkaEntity new_entity;
    ska_id_queue_dequeue(entity_id_queue, &new_entity);
    active_entity_count++;
    return new_entity;
}

void ska_ecs_entity_return(SkaEntity entity) {
    SKA_ASSERT(entity_id_queue);
    ska_id_queue_enqueue(entity_id_queue, entity);
    active_entity_count--;
}

usize ska_ecs_entity_get_active_count() {
    return active_entity_count;
}

#endif // if SKA_ECS
