#include "id_queue.h"

#include "seika1/assert.h"
#include "seika1/memory.h"

SkaIdQueue* ska_id_queue_create(usize initialCapacity) {
    SkaIdQueue* queue = SKA_ALLOC(SkaIdQueue);
    queue->capacity = initialCapacity;
    queue->size = initialCapacity;
    queue->front = 0;
    queue->rear = (SKA_QUEUE_T)initialCapacity - 1;
    queue->array = (SKA_QUEUE_T*)SKA_ALLOC_BYTES(queue->capacity * sizeof(SKA_QUEUE_T));
    for (SKA_QUEUE_T i = 0; i < initialCapacity; i++) {
        queue->array[i] = i;
    }
    return queue;
}

void ska_id_queue_destroy(SkaIdQueue* queue) {
    SKA_FREE(queue->array);
    SKA_FREE(queue);
}

bool ska_id_queue_is_full(SkaIdQueue* queue) {
    return queue->size == queue->capacity;
}

bool ska_id_queue_is_empty(SkaIdQueue* queue) {
    return queue->size == 0;
}

bool ska_id_queue_enqueue(SkaIdQueue* queue, SKA_QUEUE_T item) {
    if (ska_id_queue_is_full(queue)) {
        return false;
    }
    queue->rear = (queue->rear + 1) % queue->capacity;
    queue->array[queue->rear] = item;
    queue->size++;
    return true;
}

bool ska_id_queue_dequeue(SkaIdQueue* queue, SKA_QUEUE_T* outId) {
    // If id queue is empty recreate data array
    if (ska_id_queue_is_empty(queue)) {
        SKA_FREE(queue->array);
        const usize prevCapacity = queue->capacity;
        queue->capacity *= 2;
        queue->array = (SKA_QUEUE_T*)SKA_ALLOC_BYTES_ZEROED(queue->capacity * sizeof(SKA_QUEUE_T));
        for (SKA_QUEUE_T i = prevCapacity; i < queue->capacity; i++) {
            queue->array[i] = i;
        }

        queue->front = prevCapacity;
        queue->rear = queue->capacity - 1;
        queue->size = prevCapacity;
    }
    *outId = queue->array[queue->front];
    queue->front = (queue->front + 1) % queue->capacity;
    queue->size--;
    return true;
}

bool ska_id_queue_front(SkaIdQueue* queue, SKA_QUEUE_T* outId) {
    if (ska_id_queue_is_empty(queue) || outId == NULL) {
        return false;
    }
    *outId = queue->array[queue->front];
    return true;
}

bool ska_id_queue_rear(SkaIdQueue* queue, SKA_QUEUE_T* outId) {
    if (ska_id_queue_is_empty(queue) || outId == NULL) {
        return false;
    }
    *outId = queue->array[queue->rear];
    return true;
}
