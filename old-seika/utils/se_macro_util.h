#pragma once

#include <stdbool.h>

// Macros that are set up to run empty statements if missing a 'define'.

#ifndef DEBUG
#define SE_DEBUG_MACRO(STATEMENT) \
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
