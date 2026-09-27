#include "array2d.h"

#include <string.h>

#include "seika/memory.h"
#include "seika/assert.h"

#define SKA_ARRAY2D_IS_COORD_INVALID(ARRAY2D, ROW, COL) ((int32)ROW >= ARRAY2D->size.w || (int32)COL >= ARRAY2D->size.h)

SkaArray2D* ska_array2d_create(usize rows, usize cols, usize element_size) {
    SkaArray2D* new_array = SKA_ALLOC(SkaArray2D);
    new_array->data = SKA_ALLOC_BYTES_ZEROED(cols * sizeof(void*));
    for (usize i = 0; i < cols; i++) {
        new_array->data[i] = SKA_ALLOC_BYTES_ZEROED(rows * element_size);
    }
    new_array->size = (SkaSize2Di){ .w = (int32)rows, .h = (int32)cols };
    new_array->element_size = element_size;
    return new_array;
}

void ska_array2d_destroy(SkaArray2D* array2d) {
    for (int32 i = 0; i < array2d->size.h; i++) {
        SKA_FREE(array2d->data[i]);
    }
    SKA_FREE(array2d->data);
    SKA_FREE(array2d);
}

void* ska_array2d_get(SkaArray2D* array2d, usize x, usize y) {
    if (SKA_ARRAY2D_IS_COORD_INVALID(array2d, x, y)) {
        return NULL;
    }
    return (void*)((char*)array2d->data[y] + x * array2d->element_size);
}

bool ska_array2d_set(SkaArray2D* array2d, usize x, usize y, void* new_value) {
    if (SKA_ARRAY2D_IS_COORD_INVALID(array2d, x, y)) {
        return false;
    }
    memcpy((void*)((char*)array2d->data[y] + x * array2d->element_size), new_value, array2d->element_size);
    return true;
}

void ska_array2d_resize(SkaArray2D* array2d, usize new_x, usize new_y) {
    // Reallocate memory for data array
    const usize new_width = SKA_MATH_MAX(new_x, 1);
    const usize new_height = SKA_MATH_MAX(new_y, 1);
    // Cache off old data
    void** old_data = array2d->data;
    const SkaSize2Di old_size = array2d->size;
    // Allocate data for all columns
    array2d->data = SKA_ALLOC_BYTES_ZEROED(new_height * sizeof(void*));
    // Iterate over new rows
    for (usize i = 0; i < new_height; i++) {
        // Allocate data for new row
        array2d->data[i] = SKA_ALLOC_BYTES_ZEROED(new_x * array2d->element_size);
        if (i < (usize)old_size.h) {
            // Now copy old data
            const usize bytes_to_copy = SKA_MATH_MIN((usize)old_size.w, new_width) * array2d->element_size;
            memcpy(array2d->data[i], old_data[i], bytes_to_copy);
        }
    }

    array2d->size = (SkaSize2Di){ .w = (int32)new_x, .h = (int32)new_y };

    if (new_x == 0 && new_y == 0) {
        ska_array2d_reset(array2d);
    }

    for (usize i = 0; i < (usize)old_size.h; i++) {
        SKA_FREE(old_data[i]);
    }
    SKA_FREE(old_data);
}

void ska_array2d_clear(SkaArray2D* array2d) {
    ska_array2d_resize(array2d, 0, 0);
}

void ska_array2d_reset(SkaArray2D* array2d) {
    ska_array2d_reset_default(array2d, NULL);
}

void ska_array2d_reset_default(SkaArray2D* array2d, void* default_value) {
    if (default_value) {
        for (int32 y = 0; y < array2d->size.h; y++) {
            for (int32 x = 0; x < array2d->size.w; x++) {
                ska_array2d_set(array2d, x, y, default_value);
            }
        }
    } else {
        for (int32 i = 0; i < array2d->size.h; i++) {
            memset(array2d->data[i], 0, array2d->size.w * array2d->element_size);
        }
    }
}
