#pragma once

#include <stddef.h>
#include <stdbool.h>

#define SKA_COMMAND_LINE_ARGS_KEY_LIMIT 8
#define SKA_COMMAND_LINE_ARGS_RETURN_LIMIT 8
#define SKA_COMMAND_LINE_ARGS_RETURN_VALUES_LIMIT 8

typedef struct SKACmdLineArgDef {
    const char* id;
    const char* description;
    bool expectsValue;
    const char* keys[SKA_COMMAND_LINE_ARGS_KEY_LIMIT];
} SKACmdLineArgDef;

typedef struct SKACmdLineArgKeyResult {
    const char* id;
    const char* values[SKA_COMMAND_LINE_ARGS_RETURN_VALUES_LIMIT];
    size_t valueCount;
} SKACmdLineArgKeyResult;

typedef struct SKACmdLineArgResult {
    SKACmdLineArgKeyResult keyResults[SKA_COMMAND_LINE_ARGS_RETURN_LIMIT];
    size_t keyResultCount;
} SKACmdLineArgResult;

SKACmdLineArgResult sk_cmd_line_args_util_parse(int argv, char** args, SKACmdLineArgDef* argDefs);
void sk_cmd_line_args_util_print_results(const SKACmdLineArgResult* result);
