#include "queue.h"

#include "seika/memory.h"

SkaQueue* ska_queue_create(usize capacity, uint32 invalidValue) {
    SkaQueue* queue = SKA_MEM_ALLOCATE(SkaQueue);
    queue->capacity = capacity;
    queue->invalidValue = invalidValue;
    queue->size = 0;
    queue->front = 0;
    queue->rear = (uint32)capacity - 1;
    queue->array = (uint32*)SKA_MEM_ALLOCATE_SIZE(queue->capacity * sizeof(uint32));
    return queue;
}

void ska_queue_destroy(SkaQueue* queue) {
    SKA_MEM_FREE(queue->array);
    SKA_MEM_FREE(queue);
}

bool ska_queue_is_full(SkaQueue* queue) {
    return queue->size == queue->capacity;
}

bool ska_queue_is_empty(SkaQueue* queue) {
    return queue->size == 0;
}

void ska_queue_enqueue(SkaQueue* queue, uint32 item) {
    if (ska_queue_is_full(queue)) {
        return;
    }
    queue->rear = (queue->rear + 1) % queue->capacity;
    queue->array[queue->rear] = item;
    queue->size++;
}

uint32 ska_queue_dequeue(SkaQueue* queue) {
    if (ska_queue_is_empty(queue)) {
        return queue->invalidValue;
    }
    uint32 value = queue->array[queue->front];
    queue->front = (queue->front + 1) % queue->capacity;
    queue->size--;
    return value;
}

uint32 ska_queue_front(SkaQueue* queue) {
    if (ska_queue_is_empty(queue)) {
        return queue->invalidValue;
    }
    return queue->array[queue->front];
}

uint32 ska_queue_rear(SkaQueue* queue) {
    if (ska_queue_is_empty(queue)) {
        return queue->invalidValue;
    }
    return queue->array[queue->rear];
}
