#pragma once

#include <stdio.h>
#include <time.h>

#include "seika/defines.h"

#define SKA_PROFILE_CODE(CodeBlock)                                   \
do {                                                                 \
clock_t start_time, end_time;                                        \
f32 cpu_time_used;                                                \
start_time = clock();                                                \
CodeBlock                                                            \
end_time = clock();                                                  \
cpu_time_used = ((f32)(end_time - start_time)) / CLOCKS_PER_SEC; \
printf("Time taken: %f seconds\n", cpu_time_used);                   \
} while (0)

/*
 * // Example
 * SKA_PROFILE_CODE(
 *     for (int i = 0; i < 1000000; i++) {}
 * );
 */

#define SKA_PROFILE_CODE_WITH_VAR(CpuTimeVar, CodeBlock)             \
do {                                                                \
clock_t start_time, end_time;                                       \
start_time = clock();                                               \
CodeBlock                                                           \
end_time = clock();                                                 \
(CpuTimeVar) = ((f32)(end_time - start_time)) / CLOCKS_PER_SEC; \
} while (0)

/*
 * // Example:
 * f32 cpu_time_taken;
 * SKA_PROFILE_CODE_WITH_VAR(
 *     cpu_time_taken,
 *     for (int i = 0; i < 1000000; i++) {}
 * );
 * printf("Time taken: %f seconds\n", cpu_time_taken);
 */
