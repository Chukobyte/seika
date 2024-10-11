#pragma once

#include "seika/defines.h"

#define SKA_ARRAY_LIST_GET_ARRAY(LIST, TYPE) ((TYPE*)((LIST)->data))
#define SKA_ARRAY_LIST_FOR_EACH(LIST, TYPE, VALUE) for(TYPE* VALUE = SKA_ARRAY_LIST_GET_ARRAY(LIST, TYPE), *VALUE##_end = VALUE + (LIST)->size; VALUE < VALUE##_end; VALUE++)

// Generic array list, use 'ska_array_list_create' to allocate on the heap
typedef struct SkaArrayList {
    void* data;
    usize valueSize; // Size of each element of the data
    usize size; // How many elements are in the array
    usize capacity; // Max elements allowed in array before resizing
    usize initialCapacity; // Min capacity for the array list
} SkaArrayList;

// Will create a new array list with all it's element values set to the passed in 'valueSize'
SkaArrayList* ska_array_list_create_default_capacity(usize valueSize);
SkaArrayList* ska_array_list_create(usize valueSize, usize initialCapacity);
// Will complete destroy the array list freeing up all its memory
void ska_array_list_destroy(SkaArrayList* list);
// Inserts an item at the end of the list
void ska_array_list_push_back(SkaArrayList* list, const void* value);
// Returns an item from the list from specified index
void* ska_array_list_get(SkaArrayList* list, usize index);
// Removes the first item from the list who is equal to the passed in value
bool ska_array_list_remove(SkaArrayList* list, const void* value);
bool ska_array_list_remove_by_index(SkaArrayList* list, usize index);
// Returns true if the array list size == 0
bool ska_array_list_is_empty(SkaArrayList *list);
// Will remove all items from the array list
void ska_array_list_clear(SkaArrayList *list);
