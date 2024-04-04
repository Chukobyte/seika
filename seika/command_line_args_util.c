#include "command_line_args_util.h"

#include <stddef.h>
#include <string.h>
#include <stdio.h>

#include "seika/data_structures/hash_map_string.h"

static SkaStringHashMap* ska_cmd_line_args_get_key_to_arg_def_map(SKACmdLineArgDef* argDefs) {
    SkaStringHashMap* keyToArgDefMap = ska_string_hash_map_create_default_capacity();
    for (usize i = 0; i < 999999999; i++) {
        if (argDefs[i].id == NULL) {
            break;
        }

        SKACmdLineArgDef* argDef = &argDefs[i];
        for (usize argIndex = 0; argIndex < SKA_COMMAND_LINE_ARGS_KEY_LIMIT; argIndex++) {
            const char* argKey = argDef->keys[argIndex];
            if (!argKey) {
                break;
            }

            ska_string_hash_map_add(keyToArgDefMap, argKey, argDef, sizeof(SKACmdLineArgDef));
        }
    }

    return keyToArgDefMap;
}

SKACmdLineArgKeyResult* ska_cmd_line_args_util_find_or_add_key_result(const SKACmdLineArgDef* argDef, SKACmdLineArgResult* result) {
    for (usize i = 0; i < result->keyResultCount; i++) {
        SKACmdLineArgKeyResult* keyResult = &result->keyResults[i];
        for (usize j = 0; j < SKA_COMMAND_LINE_ARGS_KEY_LIMIT; j++) {
            if (!argDef->keys[j]) {
                break;
            }

            if (strcmp(keyResult->id, argDef->id) == 0) {
                return keyResult;
            }
        }
    }

    // If here, we don't have a key result so add one
    SKACmdLineArgKeyResult* keyResult = &result->keyResults[result->keyResultCount++];
    keyResult->id = argDef->id;

    return keyResult;
}

SKACmdLineArgResult ska_cmd_line_args_util_parse(int32 argv, char** args, SKACmdLineArgDef* argDefs) {
    SKACmdLineArgResult result = (SKACmdLineArgResult){0};
    SkaStringHashMap* keyToArgDefMap = ska_cmd_line_args_get_key_to_arg_def_map(argDefs);

    for (int32 i = 0; i < argv; i++) {
        const char* arg = args[i];
        const SKACmdLineArgDef* argDef = (SKACmdLineArgDef*)ska_string_hash_map_find(keyToArgDefMap, arg);
        if (argDef) {
            SKACmdLineArgKeyResult* keyResult = ska_cmd_line_args_util_find_or_add_key_result(argDef, &result);
            if (argDef->expectsValue) {
                const int32 nextArgIndex = i + 1;
                if (nextArgIndex < argv) {
                    keyResult->values[keyResult->valueCount++] = args[nextArgIndex];
                }
            }
        }
    }

    ska_string_hash_map_destroy(keyToArgDefMap);

    return result;
}

void ska_cmd_line_args_util_print_results(const SKACmdLineArgResult* result) {
    for (usize i = 0; i < result->keyResultCount; i++) {
        const SKACmdLineArgKeyResult* keyResult = &result->keyResults[i];
        printf("---------------------------------------------------------------\n");
        printf("Key Id: '%s'\n", keyResult->id);
        for (usize valueIndex = 0; valueIndex < keyResult->valueCount; valueIndex++) {
            printf("Value: '%s'\n", keyResult->values[valueIndex]);
        }
        printf("---------------------------------------------------------------\n");
    }
}
