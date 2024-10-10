#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "defines.h"

#define SKA_MEM_ALLOCATE(DataType)             \
(DataType*) ska_mem_allocate(sizeof(DataType))

#define SKA_MEM_ALLOCATE_ARRAY(DataType, ArraySize)             \
(DataType*) ska_mem_allocate_c(ArraySize, sizeof(DataType))

#define SKA_MEM_ALLOCATE_SIZE(Size)             \
ska_mem_allocate(Size)

#define SKA_MEM_ALLOCATE_SIZE_ZERO(Blocks, Size)             \
ska_mem_allocate_c(Blocks, Size)

#define SKA_MEM_FREE(Memory)             \
ska_mem_free(Memory)

void* ska_mem_allocate(usize size);
void* ska_mem_allocate_c(usize blocks, usize size);
void* ska_mem_reallocate(void* memory, usize size);
void ska_mem_free(void* memory);

#ifdef __cplusplus
}
#endif
