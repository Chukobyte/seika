#pragma once

#include <stddef.h>
#include <stdbool.h>

#define SK_COMMAND_LINE_ARGS_KEY_LIMIT 8
#define SK_COMMAND_LINE_ARGS_RETURN_LIMIT 8
#define SK_COMMAND_LINE_ARGS_RETURN_VALUES_LIMIT 8

typedef struct SKCmdLineArgDef {
    const char* id;
    const char* description;
    bool expectsValue;
    const char* keys[SK_COMMAND_LINE_ARGS_KEY_LIMIT];
} SKCmdLineArgDef;

typedef struct SKCmdLineArgKeyResult {
    const char* id;
    const char* values[SK_COMMAND_LINE_ARGS_RETURN_VALUES_LIMIT];
    size_t valueCount;
} SKCmdLineArgKeyResult;

typedef struct SKCmdLineArgResult {
    SKCmdLineArgKeyResult keyResults[SK_COMMAND_LINE_ARGS_RETURN_LIMIT];
    size_t keyResultCount;
} SKCmdLineArgResult;

SKCmdLineArgResult sk_cmd_line_args_util_parse(int argv, char** args, SKCmdLineArgDef* argDefs);
void sk_cmd_line_args_util_print_results(const SKCmdLineArgResult* result);
