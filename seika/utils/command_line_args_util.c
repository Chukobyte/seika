#include "command_line_args_util.h"

#include <stddef.h>
#include <string.h>
#include <stdio.h>

#include "../data_structures/se_hash_map_string.h"

SEStringHashMap* sk_cmd_line_args_get_key_to_arg_def_map(SKACmdLineArgDef* argDefs) {
    SEStringHashMap* keyToArgDefMap = se_string_hash_map_create_default_capacity();
    for (size_t i = 0; i < 999999999; i++) {
        if (argDefs[i].id == NULL) {
            break;
        }

        SKACmdLineArgDef* argDef = &argDefs[i];
        for (size_t argIndex = 0; argIndex < SKA_COMMAND_LINE_ARGS_KEY_LIMIT; argIndex++) {
            const char* argKey = argDef->keys[argIndex];
            if (!argKey) {
                break;
            }

            se_string_hash_map_add(keyToArgDefMap, argKey, argDef, sizeof(SKACmdLineArgDef));
        }
    }

    return keyToArgDefMap;
}

SKACmdLineArgKeyResult* sk_cmd_line_args_util_find_or_add_key_result(const SKACmdLineArgDef* argDef, SKACmdLineArgResult* result) {
    for (size_t i = 0; i < result->keyResultCount; i++) {
        SKACmdLineArgKeyResult* keyResult = &result->keyResults[i];
        for (size_t j = 0; j < SKA_COMMAND_LINE_ARGS_KEY_LIMIT; j++) {
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

SKACmdLineArgResult sk_cmd_line_args_util_parse(int argv, char** args, SKACmdLineArgDef* argDefs) {
    SKACmdLineArgResult result = (SKACmdLineArgResult){ .keyResults = {0}, .keyResultCount = 0 };
    SEStringHashMap* keyToArgDefMap = sk_cmd_line_args_get_key_to_arg_def_map(argDefs);

    for (int i = 0; i < argv; i++) {
        const char* arg = args[i];
        const SKACmdLineArgDef* argDef = (SKACmdLineArgDef*) se_string_hash_map_find(keyToArgDefMap, arg);
        if (argDef) {
            SKACmdLineArgKeyResult* keyResult = sk_cmd_line_args_util_find_or_add_key_result(argDef, &result);
            if (argDef->expectsValue) {
                const int nextArgIndex = i + 1;
                if (nextArgIndex < argv) {
                    keyResult->values[keyResult->valueCount++] = args[nextArgIndex];
                }
            }
        }
    }

    se_string_hash_map_destroy(keyToArgDefMap);

    return result;
}

void sk_cmd_line_args_util_print_results(const SKACmdLineArgResult* result) {
    for (size_t i = 0; i < result->keyResultCount; i++) {
        const SKACmdLineArgKeyResult* keyResult = &result->keyResults[i];
        printf("---------------------------------------------------------------\n");
        printf("Key Id: '%s'\n", keyResult->id);
        for (size_t valueIndex = 0; valueIndex < keyResult->valueCount; valueIndex++) {
            printf("Value: '%s'\n", keyResult->values[valueIndex]);
        }
        printf("---------------------------------------------------------------\n");
    }
}
