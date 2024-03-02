#include "ska_array2d.h"

#include <string.h>

#include "../memory/se_mem.h"
#include "seika/utils/se_assert.h"

#define SKA_ARRAY2D_IS_VALID_COORD(ARRAY2D, ROW, COL) (!((int)ROW >= ARRAY2D->size.w || (int)COL >= ARRAY2D->size.h))

SkaArray2D* ska_array2d_create(size_t rows, size_t cols, size_t elementSize) {
    SkaArray2D* newArray = SE_MEM_ALLOCATE(SkaArray2D);
    newArray->data = SE_MEM_ALLOCATE_SIZE(cols * sizeof(void*));
    for (size_t i = 0; i < cols; i++) {
        newArray->data[i] = SE_MEM_ALLOCATE_SIZE(rows * elementSize);
    }
    newArray->size = (SKASize2Di){ .w = (int)rows, .h = (int)cols };
    newArray->elementSize = elementSize;
    return newArray;
}

void ska_array2d_destroy(SkaArray2D* array2d) {
    for (int i = 0; i < array2d->size.h; i++) {
        SE_MEM_FREE(array2d->data[i]);
    }
    SE_MEM_FREE(array2d->data);
    SE_MEM_FREE(array2d);
}

void* ska_array2d_get(SkaArray2D* array2d, size_t x, size_t y) {
    if (!SKA_ARRAY2D_IS_VALID_COORD(array2d, x, y)) {
        return NULL;
    }
    return (void*)((char*)array2d->data[y] + x * array2d->elementSize);
}

bool ska_array2d_set(SkaArray2D* array2d, size_t x, size_t y, void* newValue) {
    if (!SKA_ARRAY2D_IS_VALID_COORD(array2d, x, y)) {
        return false;
    }
    memcpy((void*)((char*)array2d->data[y] + x * array2d->elementSize), newValue, array2d->elementSize);
    return true;
}

void ska_array2d_resize(SkaArray2D* array2d, size_t newX, size_t newY) {
    // Reallocate memory for data array
    const size_t width = SKA_MATH_MAX(newX, 1);
    const size_t height = SKA_MATH_MAX(newY, 1);

    array2d->data = ska_mem_reallocate(array2d->data, height * sizeof(void*));
    for (size_t i = 0; i < height; i++) {
        if (i < array2d->size.h) {
            // Resize existing rows
            array2d->data[i] = ska_mem_reallocate(array2d->data[i], width * array2d->elementSize);
        } else {
            // Allocate memory for new rows
            array2d->data[i] = SE_MEM_ALLOCATE_SIZE(width * array2d->elementSize);
        }
    }

    array2d->size = (SKASize2Di){ .w = (int)newX, .h = (int)newY };

    if (newX == 0 && newY == 0) {
        ska_array2d_reset(array2d);
    }
}

void ska_array2d_clear(SkaArray2D* array2d) {
    ska_array2d_resize(array2d, 0, 0);
}

void ska_array2d_reset(SkaArray2D* array2d) {
    ska_array2d_reset_default(array2d, NULL);
}

void ska_array2d_reset_default(SkaArray2D* array2d, void* defaultValue) {
    if (defaultValue) {
        for (int y = 0; y < array2d->size.h; y++) {
            for (int x = 0; x < array2d->size.w; x++) {
                ska_array2d_set(array2d, x, y, defaultValue);
            }
        }
    } else {
        for (int i = 0; i < array2d->size.h; i++) {
            memset(array2d->data[i], 0, array2d->size.w * array2d->elementSize);
        }
    }
}
