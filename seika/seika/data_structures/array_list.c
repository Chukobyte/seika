#include "array_list.h"

#include <string.h>

#include "seika/assert.h"

#define SKA_ARRAY_LIST_DEFAULT_CAPACITY 16
#define SKA_ARRAY_LIST_CAPACITY_GROWTH_RATE 1.5f

SkaArrayList ska_array_list_init_default_capacity(usize value_size) {
    return ska_array_list_init(value_size, SKA_ARRAY_LIST_DEFAULT_CAPACITY);
}

SkaArrayList ska_array_list_init(usize value_size, usize initial_capacity) {
    return ska_array_list_init_a(ska_mem_get_global_allocator(), value_size, initial_capacity);
}

SkaArrayList ska_array_list_init_a(SkaAllocator allocator, usize value_size, usize initial_capacity) {
    return (SkaArrayList){
        .allocator = allocator,
        .data = SKA_ALLOC_BYTES_ZEROED_A(allocator, initial_capacity * value_size),
        .value_size = value_size,
        .capacity = initial_capacity,
        .initial_capacity = initial_capacity,
        .size = 0
    };
}

void ska_array_list_deinit(SkaArrayList* list) {
    SKA_FREE_A(list->allocator, list->data);
    *list = (SkaArrayList){0}; // Zero out the list
}

bool ska_array_list_append(SkaArrayList* list, const void* value) {
    if (list->size >= list->capacity) {
        const usize new_capacity = (usize)((f32)list->capacity * SKA_ARRAY_LIST_CAPACITY_GROWTH_RATE);
        list->capacity = new_capacity;
        list->data = SKA_REALLOC_A(list->allocator, list->data, list->capacity * list->value_size);
        if (list->data == NULL) return false;
    }
    memcpy((char*)list->data + list->size * list->value_size, value, list->value_size);
    list->size++;
    return true;
}

void* ska_array_list_get(SkaArrayList* list, usize index) {
    SKA_ASSERT_FMT(index < list->size, "Attempting to access out of bounds index '%d", index);
    return (char*)list->data + index * list->value_size;
}

const void* ska_array_list_get_const(const SkaArrayList* list, usize index) {
    SKA_ASSERT_FMT(index < list->size, "Attempting to access out of bounds index '%d", index);
    return (char*)list->data + index * list->value_size;
}

bool ska_array_list_remove(SkaArrayList* list, const void* value) {
    usize index = 0;
    while (index < list->size) {
        if (memcmp((char*)list->data + index * list->value_size, value, list->value_size) == 0) {
            // Found the element, remove it
            for (usize i = index + 1; i < list->size; ++i) {
                memcpy((char*)list->data + (i - 1) * list->value_size, (char*)list->data + i * list->value_size, list->value_size);
            }
            list->size--;
            return true;
        }
        index++;
    }
    return false;
}

bool ska_array_list_remove2(SkaArrayList* list, const void* value, SkaArrayListCmp compare_func) {
    usize index = 0;
    while (index < list->size) {
        if (compare_func(value, (char*)list->data + index * list->value_size)) {
            // Found the element, remove it
            for (usize i = index + 1; i < list->size; ++i) {
                memcpy((char*)list->data + (i - 1) * list->value_size, (char*)list->data + i * list->value_size, list->value_size);
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
            memcpy((char*)list->data + (i - 1) * list->value_size, (char*)list->data + i * list->value_size, list->value_size);
        }
        list->size--;
        return true;
    }
    return false;
}

bool ska_array_list_has(const SkaArrayList* list, const void* value) {
    for (usize i = 0; i < list->size; i++) {
        if (memcmp((char*)list->data + i * list->value_size, value, list->value_size) == 0) {
            return true;
        }
    }
    return false;
}

bool ska_array_list_has2(const SkaArrayList* list, const void* value, SkaArrayListCmp compare_func) {
    for (usize i = 0; i < list->size; i++) {
        if (compare_func(value, (char*)list->data + i * list->value_size)) {
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

void ska_array_list_reserve(SkaArrayList* list, usize new_capacity) {
    if (new_capacity > list->capacity) {
        list->data = SKA_REALLOC_A(list->allocator, list->data, new_capacity * list->value_size);
        list->capacity = new_capacity;
    }
}

void ska_array_list_resize_zeroed(SkaArrayList* list, usize new_size) {
    if (new_size <= list->size) {
        return;
    }
    if (new_size > list->capacity) {
        usize new_capacity = list->capacity == 0 ? new_size : list->capacity;
        while (new_capacity < new_size) {
            new_capacity = (usize)((f32)new_capacity * SKA_ARRAY_LIST_CAPACITY_GROWTH_RATE) + 1;
        }
        list->data = SKA_REALLOC_A(list->allocator, list->data, new_capacity * list->value_size);
        list->capacity = new_capacity;
    }
    memset((char*)list->data + list->size * list->value_size, 0, (new_size - list->size) * list->value_size);
    list->size = new_size;
}
