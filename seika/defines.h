#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#include "seika/version_info.h"

#if defined(_WIN32) || defined(_WIN64)
#define PLATFORM_WINDOWS
#elif defined(__linux__)
#define PLATFORM_LINUX
#elif defined(__APPLE__) && defined(__MACH__)
#define PLATFORM_MACOS
#elif defined(__EMSCRIPTEN__)
#define PLATFORM_EMSCRIPTEN
#else
#define PLATFORM_UNKNOWN
#endif

#if defined(__linux__)
#define PLATFORM_LINUX
#endif

#if defined(__APPLE__) && defined(__MACH__)
#define PLATFORM_MACOS
#endif

#if defined(__EMSCRIPTEN__)
#define PLATFORM_EMSCRIPTEN
#endif

typedef size_t usize;

typedef int16_t int16;
typedef int16_t i16;
typedef int32_t int32;
typedef int32_t i32;
typedef int64_t int64;
typedef int64_t i64;

typedef uint32_t uint32;
typedef uint32_t u32;
typedef uint64_t uint64;
typedef uint64_t u64;

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
