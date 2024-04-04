#pragma once

#include <stddef.h>

#include "seika/defines.h"

#define SKA_ARRAY_UTILS_REMOVE_ARRAY_ITEM(ARRAY, ARRAY_SIZE, ELEMENT, EMPTY_VALUE) \
{                                                                                       \
const usize temp_arac_size = (ARRAY_SIZE);                                                                                        \
for (usize temp_arac_loop_index = 0; temp_arac_loop_index < temp_arac_size; temp_arac_loop_index++) {       \
if (ARRAY[temp_arac_loop_index] == ELEMENT) {                      \
ARRAY[temp_arac_loop_index] = ARRAY[temp_arac_loop_index + 1];                       \
ARRAY[temp_arac_loop_index + 1] = EMPTY_VALUE;                         \
(ARRAY_SIZE)--;                                   \
}                                                        \
if (ARRAY[temp_arac_loop_index] == EMPTY_VALUE && temp_arac_loop_index + 1 < temp_arac_size) {                      \
ARRAY[temp_arac_loop_index] = ARRAY[temp_arac_loop_index + 1];                       \
ARRAY[temp_arac_loop_index + 1] = EMPTY_VALUE;                         \
}                                                        \
}                                                        \
}

// Function to perform Selection Sort
void ska_array_utils_selection_sort_int(int32 arr[], int32 arraySize);

void ska_array_utils_remove_item_uint32(uint32 array[], usize* size, uint32 item, uint32 emptyValue);
