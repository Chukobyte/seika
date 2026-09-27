#pragma once

#include "defines.h"

#define SKA_COMMAND_LINE_ARGS_KEY_LIMIT 8
#define SKA_COMMAND_LINE_ARGS_RETURN_LIMIT 8
#define SKA_COMMAND_LINE_ARGS_RETURN_VALUES_LIMIT 8
#define SKA_COMMAND_LINE_ARGS_POSITIONAL_LIMIT 8

typedef struct SKACmdLineArgDef {
    const char* id;
    const char* description;
    bool expects_value;
    const char* keys[SKA_COMMAND_LINE_ARGS_KEY_LIMIT];
} SKACmdLineArgDef;

typedef struct SKACmdLineArgKeyResult {
    const char* id;
    const char* values[SKA_COMMAND_LINE_ARGS_RETURN_VALUES_LIMIT];
    usize value_count;
} SKACmdLineArgKeyResult;

typedef struct SKACmdLineArgResult {
    SKACmdLineArgKeyResult key_results[SKA_COMMAND_LINE_ARGS_RETURN_LIMIT];
    usize key_result_count;
    const char* positional[SKA_COMMAND_LINE_ARGS_POSITIONAL_LIMIT];
    usize positional_count;
} SKACmdLineArgResult;

// argDefs must be sentinel-terminated (last entry has .id == NULL).
// Pass (argc - 1, argv + 1, ...) to skip the program name.
SKACmdLineArgResult ska_cmd_line_args_util_parse(int32 argc, char** args, SKACmdLineArgDef* arg_defs);

// Returns the key result matching id, or NULL if the flag was not present.
const SKACmdLineArgKeyResult* ska_cmd_line_args_util_find_key_result(const SKACmdLineArgResult* result, const char* id);

void ska_cmd_line_args_util_print_results(const SKACmdLineArgResult* result);
