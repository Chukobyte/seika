#include "array_list.h"

#include <string.h>

#include "seika/memory.h"
#include "seika/assert.h"

#define SKA_ARRAY_LIST_DEFAULT_CAPACITY 16

SkaArrayList* ska_array_list_create_default_capacity(usize valueSize) {
    return ska_array_list_create(valueSize, SKA_ARRAY_LIST_DEFAULT_CAPACITY);
}

SkaArrayList* ska_array_list_create(usize valueSize, usize initialCapacity) {
    SkaArrayList* newList = SKA_ALLOC(SkaArrayList);
    newList->data = SKA_ALLOC_BYTES_ZEROED(initialCapacity * valueSize);
    newList->valueSize = valueSize;
    newList->capacity = initialCapacity;
    newList->initialCapacity = initialCapacity;
    newList->size = 0;
    return newList;
}

void ska_array_list_destroy(SkaArrayList* list) {
    SKA_FREE(list->data);
    SKA_FREE(list);
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
