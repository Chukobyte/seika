#pragma once

#include <stddef.h>
#include <stdbool.h>

#include "seika/defines.h"

// TODO: If other types are needed, create a macro to simplify queues by type

// Unsigned int queue
typedef struct SkaQueue {
    uint32 front, rear, size, invalidValue;
    usize capacity;
    uint32* array;
} SkaQueue;

SkaQueue* ska_queue_create(usize capacity, uint32 invalidValue);
void ska_queue_destroy(SkaQueue* queue);
bool ska_queue_is_full(SkaQueue* queue);
bool ska_queue_is_empty(SkaQueue* queue);
void ska_queue_enqueue(SkaQueue* queue, uint32 item);
uint32 ska_queue_dequeue(SkaQueue* queue);
uint32 ska_queue_front(SkaQueue* queue);
uint32 ska_queue_rear(SkaQueue* queue);
