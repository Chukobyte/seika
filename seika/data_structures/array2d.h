#pragma once

#include "seika/math/math.h"

typedef struct SkaArray2D {
    void** data;
    SkaSize2Di size; // w = rows, h = columns
    size_t elementSize;
} SkaArray2D;

SkaArray2D* ska_array2d_create(size_t rows, size_t cols, size_t elementSize);
void ska_array2d_destroy(SkaArray2D* array2d);
void* ska_array2d_get(SkaArray2D* array2d, size_t x, size_t y);
bool ska_array2d_set(SkaArray2D* array2d, size_t x, size_t y, void* newValue);
void ska_array2d_resize(SkaArray2D* array2d, size_t newX, size_t newY);
void ska_array2d_clear(SkaArray2D* array2d);
// Maintains elements in the arrays but zeroes out data
void ska_array2d_reset(SkaArray2D* array2d);
// Maintains elements in the arrays but copy memory from the passed in default value
void ska_array2d_reset_default(SkaArray2D* array2d, void* defaultValue);
