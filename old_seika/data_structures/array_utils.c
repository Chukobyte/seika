#include "array_utils.h"

static inline void ska_array_utils_swap(int32* xp, int32* yp) {
    int32 temp = *xp;
    *xp = *yp;
    *yp = temp;
}

// Function to perform Selection Sort
void ska_array_utils_selection_sort_int(int32 arr[], int32 arraySize) {
    int32 min_idx;

    // One by one move boundary of unsorted subarray
    for (int32 i = 0; i < arraySize - 1; i++) {

        // Find the minimum element in unsorted array
        min_idx = i;
        for (int32 j = i + 1; j < arraySize; j++) {
            if (arr[j] < arr[min_idx]) {
                min_idx = j;
            }
        }

        // Swap the found minimum element
        // with the first element
        ska_array_utils_swap(&arr[min_idx], &arr[i]);
    }
}

void ska_array_utils_remove_item_uint32(uint32 array[], usize* size, uint32 item, uint32 emptyValue) {
    SKA_ARRAY_UTILS_REMOVE_ARRAY_ITEM(array, *size, item, emptyValue);
}
