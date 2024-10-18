#if SKA_ECS

#include "entity.h"
#include "seika/data_structures/id_queue.h"
#include "seika/assert.h"

#define SKA_INITIAL_ENTITY_CAPACITY 1000

static SkaIdQueue* entityIdQueue = NULL;
static usize activeEntityCount = 0;

void ska_ecs_entity_initialize() {
    SKA_ASSERT(entityIdQueue == NULL);
    entityIdQueue = ska_id_queue_create(SKA_INITIAL_ENTITY_CAPACITY);
}

void ska_ecs_entity_finalize() {
    SKA_ASSERT(entityIdQueue);
    ska_id_queue_destroy(entityIdQueue);
    entityIdQueue = NULL;
    activeEntityCount = 0;
}

SkaEntity ska_ecs_entity_create() {
    SKA_ASSERT(entityIdQueue);
    SkaEntity newEntity;
    ska_id_queue_dequeue(entityIdQueue, &newEntity);
    activeEntityCount++;
    return newEntity;
}

void ska_ecs_entity_return(SkaEntity entity) {
    SKA_ASSERT(entityIdQueue);
    ska_id_queue_enqueue(entityIdQueue, entity);
    activeEntityCount--;
}

usize ska_ecs_entity_get_active_count() {
    return activeEntityCount;
}

#endif // if SKA_ECS
