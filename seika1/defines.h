#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#include "seika/version_info.h"

// Compilation modes

#ifndef SKA_RENDERING_MODULE
#define SKA_RENDERING_MODULE 1
#endif

#define SKA_INCLUDE_RENDERING 1
#define SKA_INCLUDE_THREAD 1
#define SKA_INCLUDE_NETWORKING 1
#define SKA_INCLUDE_DATA_STRUCTURES 1
#define SKA_INCLUDE_ASSET 1
#define SKA_INCLUDE_AUDIO 1
#define SKA_INCLUDE_ECS 1
#define SKA_INCLUDE_MATH 1

typedef size_t usize;

typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef uint32_t uint32;
typedef uint64_t uint64;

typedef float f32;
typedef double f64;

#ifndef DEBUG
#define SKA_IF_DEBUG(STATEMENT) \
do { STATEMENT } while(false)
#else
#define SE_DEBUG_MACRO(CONDITION, STATEMENT) \
do {} while (false)
#endif

// Used to define literal structs that are compatible from both c and c++
#ifndef __cplusplus
#define SKA_STRUCT_LITERAL(STRUCT_NAME) \
(STRUCT_NAME)
#else
#define SKA_STRUCT_LITERAL(STRUCT_NAME)
#endif

#ifdef __cplusplus
}
#endif
