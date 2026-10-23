#include "command_line_args_util.h"

#include <string.h>
#include <stdio.h>

#include "data_structures/hash_map_string.h"

static SkaStringHashMap* ska_cmd_line_args_get_key_to_arg_def_map(SKACmdLineArgDef* arg_defs) {
    SkaStringHashMap* key_to_arg_def_map = ska_string_hash_map_create_default_capacity();
    for (usize i = 0; i < 999999999; i++) {
        if (arg_defs[i].id == NULL) {
            break;
        }

        SKACmdLineArgDef* arg_def = &arg_defs[i];
        for (usize arg_index = 0; arg_index < SKA_COMMAND_LINE_ARGS_KEY_LIMIT; arg_index++) {
            const char* arg_key = arg_def->keys[arg_index];
            if (!arg_key) {
                break;
            }

            ska_string_hash_map_add(key_to_arg_def_map, arg_key, arg_def, sizeof(SKACmdLineArgDef));
        }
    }

    return key_to_arg_def_map;
}

SKACmdLineArgKeyResult* ska_cmd_line_args_util_find_or_add_key_result(const SKACmdLineArgDef* arg_def, SKACmdLineArgResult* result) {
    for (usize i = 0; i < result->key_result_count; i++) {
        SKACmdLineArgKeyResult* key_result = &result->key_results[i];
        for (usize j = 0; j < SKA_COMMAND_LINE_ARGS_KEY_LIMIT; j++) {
            if (!arg_def->keys[j]) {
                break;
            }

            if (strcmp(key_result->id, arg_def->id) == 0) {
                return key_result;
            }
        }
    }

    // If here, we don't have a key result so add one
    SKACmdLineArgKeyResult* key_result = &result->key_results[result->key_result_count++];
    key_result->id = arg_def->id;

    return key_result;
}

SKACmdLineArgResult ska_cmd_line_args_util_parse(int32 argc, char** args, SKACmdLineArgDef* arg_defs) {
    SKACmdLineArgResult result = (SKACmdLineArgResult){0};
    SkaStringHashMap* key_to_arg_def_map = ska_cmd_line_args_get_key_to_arg_def_map(arg_defs);

    for (int32 i = 0; i < argc; i++) {
        const char* arg = args[i];
        const SKACmdLineArgDef* arg_def = (SKACmdLineArgDef*)ska_string_hash_map_get(key_to_arg_def_map, arg);
        if (arg_def) {
            SKACmdLineArgKeyResult* key_result = ska_cmd_line_args_util_find_or_add_key_result(arg_def, &result);
            if (arg_def->expects_value) {
                const int32 next_arg_index = i + 1;
                if (next_arg_index < argc) {
                    key_result->values[key_result->value_count++] = args[next_arg_index];
                    i++;  // skip the consumed value
                }
            }
        } else if (result.positional_count < SKA_COMMAND_LINE_ARGS_POSITIONAL_LIMIT) {
            result.positional[result.positional_count++] = arg;
        }
    }

    ska_string_hash_map_deinit(key_to_arg_def_map);

    return result;
}

const SKACmdLineArgKeyResult* ska_cmd_line_args_util_find_key_result(const SKACmdLineArgResult* result, const char* id) {
    for (usize i = 0; i < result->key_result_count; i++) {
        if (strcmp(result->key_results[i].id, id) == 0) {
            return &result->key_results[i];
        }
    }
    return NULL;
}

void ska_cmd_line_args_util_print_results(const SKACmdLineArgResult* result) {
    for (usize i = 0; i < result->key_result_count; i++) {
        const SKACmdLineArgKeyResult* key_result = &result->key_results[i];
        printf("---------------------------------------------------------------\n");
        printf("Key Id: '%s'\n", key_result->id);
        for (usize value_index = 0; value_index < key_result->value_count; value_index++) {
            printf("Value: '%s'\n", key_result->values[value_index]);
        }
        printf("---------------------------------------------------------------\n");
    }
}
