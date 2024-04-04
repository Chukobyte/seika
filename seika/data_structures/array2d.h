#pragma once

#include "seika/math/math.h"

typedef struct SkaArray2D {
    void** data;
    SkaSize2Di size; // w = rows, h = columns
    usize elementSize;
} SkaArray2D;

SkaArray2D* ska_array2d_create(usize rows, usize cols, usize elementSize);
void ska_array2d_destroy(SkaArray2D* array2d);
void* ska_array2d_get(SkaArray2D* array2d, usize x, usize y);
bool ska_array2d_set(SkaArray2D* array2d, usize x, usize y, void* newValue);
void ska_array2d_resize(SkaArray2D* array2d, usize newX, usize newY);
void ska_array2d_clear(SkaArray2D* array2d);
// Maintains elements in the arrays but zeroes out data
void ska_array2d_reset(SkaArray2D* array2d);
// Maintains elements in the arrays but copy memory from the passed in default value
void ska_array2d_reset_default(SkaArray2D* array2d, void* defaultValue);
