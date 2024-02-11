#include "entity.h"
#include "seika/data_structures/se_queue.h"
#include "seika/utils/se_assert.h"

static SEQueue* entityIdQueue = NULL;
static SkaEntity entityIdCounter = 0;
static size_t activeEntityCount = 0;

void ska_ecs_entity_initialize() {
    SE_ASSERT(entityIdQueue == NULL);
    entityIdQueue = se_queue_create(SKA_MAX_ENTITIES, SKA_NULL_ENTITY);
    while (entityIdCounter < 100) {
        se_queue_enqueue(entityIdQueue, entityIdCounter++);
    }
}

void ska_ecs_entity_finalize() {
    SE_ASSERT(entityIdQueue);
    se_queue_destroy(entityIdQueue);
    entityIdQueue = NULL;
    entityIdCounter = 0;
    activeEntityCount = 0;
}

SkaEntity ska_ecs_entity_create() {
    SE_ASSERT(entityIdQueue);
    if (se_queue_is_empty(entityIdQueue)) {
        SE_ASSERT_FMT(entityIdCounter < SKA_MAX_ENTITIES, "Reached maxed entities ids to create, considering increasing SKA_MAX_ENTITIES!");
        se_queue_enqueue(entityIdQueue, entityIdCounter++);
    }
    activeEntityCount++;
    return se_queue_dequeue(entityIdQueue);
}

void ska_ecs_entity_return(SkaEntity entity) {
    SE_ASSERT(entityIdQueue);
    se_queue_enqueue(entityIdQueue,entity);
    activeEntityCount--;
}

size_t ska_ecs_entity_get_active_count() {
    return activeEntityCount;
}
