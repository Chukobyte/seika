#pragma once

#include <stdbool.h>

#include "array_utils.h"

// Helper class for generalized use of static arrays

#define SKA_STATIC_ARRAY_CREATE(ARRAY_TYPE, ARRAY_SIZE, ARRAY_NAME) \
static ARRAY_TYPE ARRAY_NAME[ARRAY_SIZE];                           \
static usize ARRAY_NAME ##_count = 0

#define SKA_STATIC_ARRAY_FOR_LOOP(ARRAY_NAME) \
for (usize ARRAY_NAME ##_loop_index = 0; ARRAY_NAME ##_loop_index < ARRAY_NAME ##_count; ARRAY_NAME ##_loop_index++)

#define SKA_STATIC_ARRAY_ADD(ARRAY_NAME, ARRAY_VALUE) \
ARRAY_NAME[ARRAY_NAME ##_count++] = ARRAY_VALUE

#define SKA_STATIC_ARRAY_ADD_IF_UNIQUE(ARRAY_NAME, ARRAY_VALUE) \
{                                                               \
bool ARRAY_NAME ##_has_duplicate = false;                                                               \
for (usize ARRAY_NAME ##_loop_index = 0; ARRAY_NAME ##_loop_index < ARRAY_NAME ##_count; ARRAY_NAME ##_loop_index++) { \
if (ARRAY_NAME[ARRAY_NAME ##_loop_index] == ARRAY_VALUE) {     \
ARRAY_NAME ##_has_duplicate = true;                                                               \
break;                                                               \
}\
}                                                               \
if (!ARRAY_NAME ##_has_duplicate) {                                                               \
SKA_STATIC_ARRAY_ADD(ARRAY_NAME, ARRAY_VALUE);                                                               \
}                                                               \
}

#define SKA_STATIC_ARRAY_REMOVE(ARRAY_NAME, ARRAY_VALUE, EMPTY_VALUE) \
{                                                                     \
const usize ARRAY_NAME ##_count_ref = ARRAY_NAME ##_count;           \
for (usize ARRAY_NAME ##_loop_index = 0; ARRAY_NAME ##_loop_index < ARRAY_NAME ##_count_ref; ARRAY_NAME ##_loop_index++) {       \
if (ARRAY_NAME[ARRAY_NAME ##_loop_index] == ARRAY_VALUE) {                      \
ARRAY_NAME[ARRAY_NAME ##_loop_index] = ARRAY_NAME[ARRAY_NAME ##_loop_index + 1];                       \
ARRAY_NAME[ARRAY_NAME ##_loop_index + 1] = EMPTY_VALUE;                         \
ARRAY_NAME ##_count--;                                   \
}                                                        \
if (ARRAY_NAME[ARRAY_NAME ##_loop_index] == EMPTY_VALUE && ARRAY_NAME ##_loop_index + 1 < ARRAY_NAME ##_count_ref) {                      \
ARRAY_NAME[ARRAY_NAME ##_loop_index] = ARRAY_NAME[ARRAY_NAME ##_loop_index + 1];                       \
ARRAY_NAME[ARRAY_NAME ##_loop_index + 1] = EMPTY_VALUE;                         \
}                                                        \
}                                                        \
}

#define SKA_STATIC_ARRAY_REMOVE_WITH_PREDICATE(ARRAY_NAME, ARRAY_VALUE, EMPTY_VALUE, ARRAY_TYPE, PRED_FUNC) \
{                                                                                                           \
bool (*static_array_compare_func)(ARRAY_TYPE first, ARRAY_TYPE second) = PRED_FUNC; \
const usize ARRAY_NAME ##_count_ref = ARRAY_NAME ##_count;           \
for (usize ARRAY_NAME ##_loop_index = 0; ARRAY_NAME ##_loop_index < ARRAY_NAME ##_count_ref; ARRAY_NAME ##_loop_index++) {       \
if (static_array_compare_func(ARRAY_NAME[ARRAY_NAME ##_loop_index], ARRAY_VALUE)) {                      \
ARRAY_NAME[ARRAY_NAME ##_loop_index] = ARRAY_NAME[ARRAY_NAME ##_loop_index + 1];                       \
ARRAY_NAME[ARRAY_NAME ##_loop_index + 1] = EMPTY_VALUE;                         \
ARRAY_NAME ##_count--;                                   \
}                                                        \
if (ARRAY_NAME[ARRAY_NAME ##_loop_index] == EMPTY_VALUE && ARRAY_NAME ##_loop_index + 1 < ARRAY_NAME ##_count_ref) {                      \
ARRAY_NAME[ARRAY_NAME ##_loop_index] = ARRAY_NAME[ARRAY_NAME ##_loop_index + 1];                       \
ARRAY_NAME[ARRAY_NAME ##_loop_index + 1] = EMPTY_VALUE;                         \
}                                                        \
}                                                        \
}

#define SKA_STATIC_ARRAY_EMPTY(ARRAY_NAME) \
ARRAY_NAME ##_count = 0;

#define SKA_STATIC_ARRAY_SIZE(ARRAY_NAME) (ARRAY_NAME ##_count)

#define SKA_STATIC_ARRAY_SORT_INT(ARRAY_NAME) \
if (SKA_STATIC_ARRAY_SIZE(ARRAY_NAME) > 0) {  \
ska_array_utils_selection_sort_int(ARRAY_NAME, (int32)SKA_STATIC_ARRAY_SIZE(ARRAY_NAME)); \
}

// Array Utils (TODO: Move in own file)
#define SKA_ARRAY_REMOVE_AND_CONDENSE(ARRAY_NAME, ARRAY_SIZE_VAR, ARRAY_VALUE, EMPTY_VALUE) \
{                                                                                       \
const usize temp_arac_size = ARRAY_SIZE_VAR;                                                                                        \
for (usize temp_arac_loop_index = 0; temp_arac_loop_index < temp_arac_size; temp_arac_loop_index++) {       \
if (ARRAY_NAME[temp_arac_loop_index] == ARRAY_VALUE) {                      \
ARRAY_NAME[temp_arac_loop_index] = ARRAY_NAME[temp_arac_loop_index + 1];                       \
ARRAY_NAME[temp_arac_loop_index + 1] = EMPTY_VALUE;                         \
ARRAY_SIZE_VAR--;                                   \
}                                                        \
if (ARRAY_NAME[temp_arac_loop_index] == EMPTY_VALUE && temp_arac_loop_index + 1 < temp_arac_size) {                      \
ARRAY_NAME[temp_arac_loop_index] = ARRAY_NAME[temp_arac_loop_index + 1];                       \
ARRAY_NAME[temp_arac_loop_index + 1] = EMPTY_VALUE;                         \
}                                                        \
}                                                        \
}

#define SKA_ARRAY_REMOVE_AND_CONDENSE_PREDICATE(ARRAY_NAME, ARRAY_SIZE_VAR, ARRAY_VALUE, EMPTY_VALUE, ARRAY_TYPE, PRED_FUNC) \
{                                                                                                                \
bool (*static_array_compare_func)(ARRAY_TYPE first, ARRAY_TYPE second) = PRED_FUNC; \
const usize temp_arac_size = ARRAY_SIZE_VAR;                                                                                        \
for (usize temp_arac_loop_index = 0; temp_arac_loop_index < temp_arac_size; temp_arac_loop_index++) {       \
if (static_array_compare_func(ARRAY_NAME[temp_arac_loop_index], ARRAY_VALUE)) {                      \
ARRAY_NAME[temp_arac_loop_index] = ARRAY_NAME[temp_arac_loop_index + 1];                       \
ARRAY_NAME[temp_arac_loop_index + 1] = EMPTY_VALUE;                         \
ARRAY_SIZE_VAR--;                                   \
}                                                        \
if (ARRAY_NAME[temp_arac_loop_index] == EMPTY_VALUE && temp_arac_loop_index + 1 < temp_arac_size) {                      \
ARRAY_NAME[temp_arac_loop_index] = ARRAY_NAME[temp_arac_loop_index + 1];                       \
ARRAY_NAME[temp_arac_loop_index + 1] = EMPTY_VALUE;                         \
}                                                        \
}                                                        \
}
