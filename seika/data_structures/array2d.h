#pragma once

#include "seika/math/math.h"

typedef struct SkaArray2D {
    void** data;
    SkaSize2Di size; // w = rows, h = columns
    usize element_size;
} SkaArray2D;

SkaArray2D* ska_array2d_create(usize rows, usize cols, usize element_size);
void ska_array2d_destroy(SkaArray2D* array2d);
void* ska_array2d_get(SkaArray2D* array2d, usize x, usize y);
bool ska_array2d_set(SkaArray2D* array2d, usize x, usize y, void* new_value);
void ska_array2d_resize(SkaArray2D* array2d, usize new_x, usize new_y);
void ska_array2d_clear(SkaArray2D* array2d);
// Maintains elements in the arrays but zeroes out data
void ska_array2d_reset(SkaArray2D* array2d);
// Maintains elements in the arrays but copy memory from the passed in default value
void ska_array2d_reset_default(SkaArray2D* array2d, void* default_value);
