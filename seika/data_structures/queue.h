#pragma once

#include <stddef.h>
#include <stdbool.h>

#include "seika/defines.h"

// TODO: If other types are needed, create a macro to simplify queues by type

// Unsigned int queue
typedef struct SkaIdQueue {
    uint32 front, rear, size, invalidValue;
    usize capacity;
    uint32* array;
} SkaIdQueue;

SkaIdQueue* ska_queue_create(usize capacity, uint32 invalidValue);
void ska_queue_destroy(SkaIdQueue* queue);
bool ska_queue_is_full(SkaIdQueue* queue);
bool ska_queue_is_empty(SkaIdQueue* queue);
void ska_queue_enqueue(SkaIdQueue* queue, uint32 item);
uint32 ska_queue_dequeue(SkaIdQueue* queue);
uint32 ska_queue_front(SkaIdQueue* queue);
uint32 ska_queue_rear(SkaIdQueue* queue);
