#pragma once

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SE_MEM_ALLOCATE(DataType)             \
(DataType*) ska_mem_allocate(sizeof(DataType))

#define SE_MEM_ALLOCATE_ARRAY(DataType, ArraySize)             \
(DataType*) ska_mem_allocate_c(ArraySize, sizeof(DataType))

#define SE_MEM_ALLOCATE_SIZE(Size)             \
ska_mem_allocate(Size)

#define SE_MEM_ALLOCATE_SIZE_ZERO(Blocks, Size)             \
ska_mem_allocate_c(Blocks, Size)

#define SE_MEM_FREE(Memory)             \
ska_mem_free(Memory)

void* ska_mem_allocate(size_t size);
void* ska_mem_allocate_c(size_t blocks, size_t size);
void* ska_mem_reallocate(void* memory, size_t size);
void ska_mem_free(void* memory);

#ifdef __cplusplus
}
#endif
