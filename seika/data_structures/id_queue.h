#pragma once

#define SKA_QUEUE_T uint32

#include "seika/defines.h"

// Unsigned int queue
typedef struct SkaIdQueue {
    usize front, rear, size;
    usize capacity;
    SKA_QUEUE_T* array;
} SkaIdQueue;

SkaIdQueue* ska_id_queue_create(usize initialCapacity);
void ska_id_queue_destroy(SkaIdQueue* queue);
bool ska_id_queue_is_full(SkaIdQueue* queue);
bool ska_id_queue_is_empty(SkaIdQueue* queue);
bool ska_id_queue_enqueue(SkaIdQueue* queue, SKA_QUEUE_T item);
bool ska_id_queue_dequeue(SkaIdQueue* queue, SKA_QUEUE_T* outId);
bool ska_id_queue_front(SkaIdQueue* queue, SKA_QUEUE_T* outId);
bool ska_id_queue_rear(SkaIdQueue* queue, SKA_QUEUE_T* outId);
