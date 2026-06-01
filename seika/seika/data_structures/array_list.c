#include "array_list.h"

#include <string.h>

#include "seika/assert.h"

#define SKA_ARRAY_LIST_DEFAULT_CAPACITY 16
#define SKA_ARRAY_LIST_CAPACITY_GROWTH_RATE 1.5f

SkaArrayList ska_array_list_init_default_capacity(usize valueSize) {
    return ska_array_list_init(valueSize, SKA_ARRAY_LIST_DEFAULT_CAPACITY);
}

SkaArrayList ska_array_list_init(usize valueSize, usize initialCapacity) {
    return ska_array_list_init_a(ska_mem_get_global_allocator(), valueSize, initialCapacity);
}

SkaArrayList ska_array_list_init_a(SkaAllocator allocator, usize valueSize, usize initialCapacity) {
    return (SkaArrayList){
        .allocator = allocator,
        .data = SKA_ALLOC_BYTES_ZEROED_A(allocator, initialCapacity * valueSize),
        .valueSize = valueSize,
        .capacity = initialCapacity,
        .initialCapacity = initialCapacity,
        .size = 0
    };
}

void ska_array_list_deinit(SkaArrayList* list) {
    SKA_FREE_A(list->allocator, list->data);
    *list = (SkaArrayList){0}; // Zero out the list
}

bool ska_array_list_append(SkaArrayList* list, const void* value) {
    if (list->size >= list->capacity) {
        const usize newCapacity = (usize)((f32)list->capacity * SKA_ARRAY_LIST_CAPACITY_GROWTH_RATE);
        list->capacity = newCapacity;
        list->data = SKA_REALLOC_A(list->allocator, list->data, list->capacity * list->valueSize);
        if (list->data == NULL) return false;
    }
    memcpy((char*)list->data + list->size * list->valueSize, value, list->valueSize);
    list->size++;
    return true;
}

void* ska_array_list_get(SkaArrayList* list, usize index) {
    SKA_ASSERT_FMT(index < list->size, "Attempting to access out of bounds index '%d", index);
    return (char*)list->data + index * list->valueSize;
}

const void* ska_array_list_get_const(const SkaArrayList* list, usize index) {
    SKA_ASSERT_FMT(index < list->size, "Attempting to access out of bounds index '%d", index);
    return (char*)list->data + index * list->valueSize;
}

bool ska_array_list_remove(SkaArrayList* list, const void* value) {
    usize index = 0;
    while (index < list->size) {
        if (memcmp((char*)list->data + index * list->valueSize, value, list->valueSize) == 0) {
            // Found the element, remove it
            for (usize i = index + 1; i < list->size; ++i) {
                memcpy((char*)list->data + (i - 1) * list->valueSize, (char*)list->data + i * list->valueSize, list->valueSize);
            }
            list->size--;
            return true;
        }
        index++;
    }
    return false;
}

bool ska_array_list_remove2(SkaArrayList* list, const void* value, SkaArrayListCmp compareFunc) {
    usize index = 0;
    while (index < list->size) {
        if (compareFunc(value, (char*)list->data + index * list->valueSize)) {
            // Found the element, remove it
            for (usize i = index + 1; i < list->size; ++i) {
                memcpy((char*)list->data + (i - 1) * list->valueSize, (char*)list->data + i * list->valueSize, list->valueSize);
            }
            list->size--;
            return true;
        }
        index++;
    }
    return false;
}

bool ska_array_list_remove_by_index(SkaArrayList* list, usize index) {
    if (index < list->size) {
        // Shift elements after the removed element
        for (usize i = index + 1; i < list->size; ++i) {
            memcpy((char*)list->data + (i - 1) * list->valueSize, (char*)list->data + i * list->valueSize, list->valueSize);
        }
        list->size--;
        return true;
    }
    return false;
}

bool ska_array_list_has(const SkaArrayList* list, const void* value) {
    for (usize i = 0; i < list->size; i++) {
        if (memcmp((char*)list->data + i * list->valueSize, value, list->valueSize) == 0) {
            return true;
        }
    }
    return false;
}

bool ska_array_list_has2(const SkaArrayList* list, const void* value, SkaArrayListCmp compareFunc) {
    for (usize i = 0; i < list->size; i++) {
        if (compareFunc(value, (char*)list->data + i * list->valueSize)) {
            return true;
        }
    }
    return false;
}

bool ska_array_list_is_empty(const SkaArrayList *list) {
    return list->size == 0;
}

void ska_array_list_clear(SkaArrayList *list) {
    list->size = 0;
}

void ska_array_list_reserve(SkaArrayList* list, usize newCapacity) {
    if (newCapacity > list->capacity) {
        list->data = SKA_REALLOC_A(list->allocator, list->data, newCapacity * list->valueSize);
        list->capacity = newCapacity;
    }
}
