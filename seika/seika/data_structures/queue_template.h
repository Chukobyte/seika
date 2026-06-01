// Test template

#ifndef SKA_QUEUE_TEMPLATE
#error "SKA_QUEUE_TEMPLATE must be defined before including queue_template.h"
#endif

#include "seika/defines.h"
#include "seika/memory.h"

#define SKA_QUEUE_T(Name) SkaQueue_##Name
#define SKA_QUEUE_FUNC(Name, Suffix) ska_queue_##Name##_##Suffix

// Struct definition
typedef struct SKA_QUEUE_T(SKA_QUEUE_TEMPLATE) {
    SKA_QUEUE_TEMPLATE front, rear, size, invalidValue;
    usize capacity;
    SKA_QUEUE_TEMPLATE* array;
} SKA_QUEUE_T(SKA_QUEUE_TEMPLATE);

// Function definitions
SKA_QUEUE_T(SKA_QUEUE_TEMPLATE)* SKA_QUEUE_FUNC(SKA_QUEUE_TEMPLATE, create)(usize capacity, SKA_QUEUE_TEMPLATE invalidValue) {
    SKA_QUEUE_T(SKA_QUEUE_TEMPLATE)* queue = SKA_ALLOC(SKA_QUEUE_T(SKA_QUEUE_TEMPLATE));
    queue->capacity = capacity;
    queue->invalidValue = invalidValue;
    queue->size = 0;
    queue->front = 0;
    queue->rear = capacity - 1;
    queue->array = (SKA_QUEUE_TEMPLATE*)SKA_ALLOC_BYTES(queue->capacity * sizeof(SKA_QUEUE_TEMPLATE));
    return queue;
}

void SKA_QUEUE_FUNC(SKA_QUEUE_TEMPLATE, destroy)(SKA_QUEUE_T(SKA_QUEUE_TEMPLATE)* queue) {
    SKA_FREE(queue->array);
    SKA_FREE(queue);
}

bool SKA_QUEUE_FUNC(SKA_QUEUE_TEMPLATE, is_full)(SKA_QUEUE_T(SKA_QUEUE_TEMPLATE)* queue) {
    return queue->size == queue->capacity;
}

bool SKA_QUEUE_FUNC(SKA_QUEUE_TEMPLATE, is_empty)(SKA_QUEUE_T(SKA_QUEUE_TEMPLATE)* queue) {
    return queue->size == 0;
}

void SKA_QUEUE_FUNC(SKA_QUEUE_TEMPLATE, enqueue)(SKA_QUEUE_T(SKA_QUEUE_TEMPLATE)* queue, SKA_QUEUE_TEMPLATE item) {
    if (SKA_QUEUE_FUNC(SKA_QUEUE_TEMPLATE, is_full)(queue)) {
        return;
    }
    queue->rear = (queue->rear + 1) % queue->capacity;
    queue->array[queue->rear] = item;
    queue->size++;
}

SKA_QUEUE_TEMPLATE SKA_QUEUE_FUNC(SKA_QUEUE_TEMPLATE, dequeue)(SKA_QUEUE_T(SKA_QUEUE_TEMPLATE)* queue) {
    if (SKA_QUEUE_FUNC(SKA_QUEUE_TEMPLATE, is_empty)(queue)) {
        return queue->invalidValue;
    }
    SKA_QUEUE_TEMPLATE item = queue->array[queue->front];
    queue->front = (queue->front + 1) % queue->capacity;
    queue->size--;
    return item;
}

SKA_QUEUE_TEMPLATE SKA_QUEUE_FUNC(SKA_QUEUE_TEMPLATE, front)(SKA_QUEUE_T(SKA_QUEUE_TEMPLATE)* queue) {
    if (SKA_QUEUE_FUNC(SKA_QUEUE_TEMPLATE, is_empty)(queue)) {
        return queue->invalidValue;
    }
    return queue->array[queue->front];
}

SKA_QUEUE_TEMPLATE SKA_QUEUE_FUNC(SKA_QUEUE_TEMPLATE, rear)(SKA_QUEUE_T(SKA_QUEUE_TEMPLATE)* queue) {
    if (SKA_QUEUE_FUNC(SKA_QUEUE_TEMPLATE, is_empty)(queue)) {
        return queue->invalidValue;
    }
    return queue->array[queue->rear];
}

#undef SKA_QUEUE_T
#undef SKA_QUEUE_FUNC
#undef SKA_QUEUE_TEMPLATE
