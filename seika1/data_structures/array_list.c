#include "array_list.h"

#include <string.h>

#include "seika/memory.h"
#include "seika/assert.h"

#define SKA_ARRAY_LIST_DEFAULT_CAPACITY 16

SkaArrayList* ska_array_list_create_default_capacity(usize valueSize) {
    return ska_array_list_create(valueSize, SKA_ARRAY_LIST_DEFAULT_CAPACITY);
}

SkaArrayList* ska_array_list_create(usize valueSize, usize initialCapacity) {
    SkaArrayList* newList = SKA_MEM_ALLOCATE(SkaArrayList);
    newList->data = SKA_MEM_ALLOCATE_SIZE(initialCapacity * valueSize);
    newList->valueSize = valueSize;
    newList->capacity = initialCapacity;
    newList->initialCapacity = initialCapacity;
    return newList;
}

void ska_array_list_destroy(SkaArrayList* list) {
    SKA_MEM_FREE(list->data);
    SKA_MEM_FREE(list);
}

void ska_array_list_push_back(SkaArrayList* list, const void* value) {
    if (list->size >= list->capacity) {
        list->capacity *= 2;
        list->data = ska_mem_reallocate(list->data, list->capacity * list->valueSize);
    }
    memcpy((char*)list->data + list->size * list->valueSize, value, list->valueSize);
    list->size++;
}

void* ska_array_list_get(SkaArrayList* list, usize index) {
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
        } else {
            index++;
        }
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

bool ska_array_list_is_empty(SkaArrayList *list) {
    return list->size == 0;
}

void ska_array_list_clear(SkaArrayList *list) {
    list->size = 0;
}
