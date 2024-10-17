#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#include "seika1/version_info.h"

// Compilation modes

#ifndef SKA_ECS
#define SKA_ECS 1
#endif

#ifndef SKA_INPUT
#define SKA_INPUT 1
#endif

#ifndef SKA_RENDERING
#define SKA_RENDERING 1
#endif

#ifndef SKA_AUDIO
#define SKA_AUDIO 1
#endif

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
