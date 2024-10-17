#include "entity.h"
#include "seika/data_structures/queue.h"
#include "seika/assert.h"

static SkaQueue* entityIdQueue = NULL;
static SkaEntity entityIdCounter = 0;
static usize activeEntityCount = 0;

void ska_ecs_entity_initialize() {
    SKA_ASSERT(entityIdQueue == NULL);
    entityIdQueue = ska_queue_create(SKA_MAX_ENTITIES, SKA_NULL_ENTITY);
    while (entityIdCounter < 100) {
        ska_queue_enqueue(entityIdQueue, entityIdCounter++);
    }
}

void ska_ecs_entity_finalize() {
    SKA_ASSERT(entityIdQueue);
    ska_queue_destroy(entityIdQueue);
    entityIdQueue = NULL;
    entityIdCounter = 0;
    activeEntityCount = 0;
}

SkaEntity ska_ecs_entity_create() {
    SKA_ASSERT(entityIdQueue);
    if (ska_queue_is_empty(entityIdQueue)) {
        SKA_ASSERT_FMT(entityIdCounter < SKA_MAX_ENTITIES, "Reached maxed entities ids to create, considering increasing SKA_MAX_ENTITIES!");
        ska_queue_enqueue(entityIdQueue, entityIdCounter++);
    }
    activeEntityCount++;
    return ska_queue_dequeue(entityIdQueue);
}

void ska_ecs_entity_return(SkaEntity entity) {
    SKA_ASSERT(entityIdQueue);
    ska_queue_enqueue(entityIdQueue,entity);
    activeEntityCount--;
}

usize ska_ecs_entity_get_active_count() {
    return activeEntityCount;
}
