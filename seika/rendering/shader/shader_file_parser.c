#if SKA_RENDERING

#include "shader_file_parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "shader_source.h"
#include "seika/string.h"
#include "seika/memory.h"

#define SHADER_FILE_PARSER_ERROR_RETURN(RESULT, SOURCE, MESSAGE) \
ska_strcpy((RESULT).error_message, (MESSAGE));                        \
SKA_FREE((SOURCE));                                           \
return (RESULT);

#define SHADER_FILE_PARSER_ERROR_FMT_RETURN(RESULT, SOURCE, FMT, ...) \
sprintf((RESULT).error_message, (FMT), ##__VA_ARGS__);                 \
SKA_FREE((SOURCE));                                                \
return (RESULT);

static char* shader_file_parse_data_get_full_uniforms_source(SkaShaderFileParseData* parse_data) {
  if (parse_data->uniform_count == 0) {
    return NULL;
  }
    char uniforms_buffer[1024];
    uniforms_buffer[0] = '\0';
    for (usize i = 0; i < parse_data->uniform_count; i++) {
      ska_strcat(uniforms_buffer, "uniform ");
      switch (parse_data->uniforms[i].type) {
      case SkaShaderParamType_BOOL: {
        ska_strcat(uniforms_buffer, "bool ");
        break;
      }
      case SkaShaderParamType_INT: {
        ska_strcat(uniforms_buffer, "int ");
        break;
      }
      case SkaShaderParamType_FLOAT: {
        ska_strcat(uniforms_buffer, "float ");
        break;
      }
      case SkaShaderParamType_FLOAT2: {
        ska_strcat(uniforms_buffer, "vec2 ");
        break;
      }
      case SkaShaderParamType_FLOAT3: {
        ska_strcat(uniforms_buffer, "vec3 ");
        break;
      }
      case SkaShaderParamType_FLOAT4: {
        ska_strcat(uniforms_buffer, "vec4 ");
        break;
      }
      default:
        break;
      }
      ska_strcat(uniforms_buffer, parse_data->uniforms[i].name);
      ska_strcat(uniforms_buffer, ";\n");
    }
    return ska_strdup(uniforms_buffer);
}

char* shader_file_parse_data_get_full_functions_source(SkaShaderFileParseData* parse_data) {
  if (parse_data->function_count == 0) {
    return NULL;
  }
    char functions_buffer[4096];
    functions_buffer[0] = '\0';
    for (usize i = 0; i < parse_data->function_count; i++) {
      ska_strcat(functions_buffer,
                 parse_data->functions[i].full_function_source);
      ska_strcat(functions_buffer, "\n");
    }
    return ska_strdup(functions_buffer);
}

void shader_file_parse_data_delete_internal_memory(SkaShaderFileParseData* parse_data) {
  SKA_FREE(parse_data->full_vertex_source);
  SKA_FREE(parse_data->full_fragment_source);
  for (usize i = 0; i < parse_data->function_count; i++) {
    SKA_FREE(parse_data->functions[i].name);
    SKA_FREE(parse_data->functions[i].full_function_source);
  }
}

// Will parse the next token, returns true if there is more source to parse
bool shader_file_find_next_token(char** shader_source, char* token_out, bool* semi_colon_found) {
    token_out[0] = '\0';

    if (semi_colon_found) {
        *semi_colon_found = false;
    }

    int token_index = 0;
    while (*(*shader_source) != '\0') {
        if (*(*shader_source) == ' ' || *(*shader_source) == ';') {
            if (semi_colon_found && *(*shader_source) == ';') {
                *semi_colon_found = true;
            }
            (*shader_source)++;
            token_out[token_index] = '\0';
            return true;
        }
        if (*(*shader_source) != '\n') {
            token_out[token_index++] = *(*shader_source);
        }
        (*shader_source)++;
    }
    return false;
}

// Continue parsing shader source and returns 'true' if an '=' was found
bool shader_file_find_next_uniform_equals_token(char** shader_source) {
    while (*(*shader_source) != '\0') {
        if (*(*shader_source) == '=') {
            (*shader_source)++;
            return true;
        }
        (*shader_source)++;
    }
    return false;
}

bool shader_file_find_next_uniform_default_value(char** shader_source, char* token_out, bool* semi_colon_found) {
    token_out[0] = '\0';

    if (semi_colon_found) {
        *semi_colon_found = false;
    }

    int token_index = 0;
    while (*(*shader_source) != '\0') {
        if (*(*shader_source) == ';') {
            if (semi_colon_found) {
                *semi_colon_found = true;
            }
            (*shader_source)++;
            token_out[token_index] = '\0';
            return true;
        }
        if (*(*shader_source) != '\n' && *(*shader_source) != ' ') {
            token_out[token_index++] = *(*shader_source);
        }
        (*shader_source)++;
    }
    return false;
}

SkaShaderFileParserFunction shader_file_find_next_function(char** shader_source, const char* function_return_type) {
  SkaShaderFileParserFunction parsed_function = {.name = NULL,
                                                 .full_function_source = NULL};
  char shader_function_buffer[1024];
  ska_strcpy(shader_function_buffer, function_return_type);
  ska_strcat(shader_function_buffer, " ");
  usize buffer_index = strlen(shader_function_buffer);

  // Get name first
  char shader_function_name[64];
  unsigned int shader_function_name_index = 0;
  while (*(*shader_source) != '\0') {
    shader_function_buffer[buffer_index++] = *(*shader_source);
    if (*(*shader_source) != ' ') {
      shader_function_name[shader_function_name_index++] = *(*shader_source);
    }
    (*shader_source)++;
    if (*(*shader_source) == '(') {
      shader_function_name[shader_function_name_index] = '\0';
      break;
    }
    }
    // Now just loop through until we encounter a '}'
    usize open_brackets_count = 0;
    while (*(*shader_source) != '\0') {
        shader_function_buffer[buffer_index++] = *(*shader_source);
        (*shader_source)++;
        if (*(*shader_source) == '{') {
            open_brackets_count++;
        } else if (*(*shader_source) == '}') {
            shader_function_buffer[buffer_index++] = *(*shader_source);
            (*shader_source)++;
            open_brackets_count--;
            if (open_brackets_count == 0) {
                shader_function_buffer[buffer_index] = '\0';
                break;
            }
        }
    }

    parsed_function.name = ska_strdup(shader_function_name);
    parsed_function.full_function_source = ska_strdup(shader_function_buffer);
    return parsed_function;
}

// Should have a valid function at this point...
char* shader_file_parse_function_body(const char* function_source) {
    char shader_function_buffer[1024];
    shader_function_buffer[0] = '\0';
    ska_strcpy(shader_function_buffer, function_source);
    unsigned int function_buffer_index = 0;
    char current_token = shader_function_buffer[function_buffer_index];
    // Find beginning body
    while (current_token != '{') {
        current_token = shader_function_buffer[++function_buffer_index];
    }
    char function_body_buffer[1024];
    unsigned int function_body_index = 0;
    current_token = shader_function_buffer[++function_buffer_index];
    // Return NULL if an empty function with (with no newlines at least)
    if (current_token == '}') {
        return NULL;
    }
    usize open_brackets_count = 1;
    while (open_brackets_count > 0) {
        current_token = shader_function_buffer[function_buffer_index++];
        function_body_buffer[function_body_index++] = current_token;
        if (current_token == '{') {
            open_brackets_count++;
        } else if (current_token == '}') {
            open_brackets_count--;
        }
    }
    function_body_buffer[function_body_index - 1] = '\n';
    function_body_buffer[function_body_index] = '\0';

    return ska_strdup(function_body_buffer);
}

typedef struct ShaderFileParseVecParseResult {
    SkaVector4 vector;
    char error_message[64];
} ShaderFileParseVecParseResult;

// TODO: Refactor and make shorter...
ShaderFileParseVecParseResult shader_file_parse_vec_default_value_token(const char* token) {
  ShaderFileParseVecParseResult result = {
      .vector = {.x = 0.0f, .y = 0.0f, .z = 0.0f, .w = 0.0f},
      .error_message = {0}};
  unsigned int source_token_index = 4;
  char current_float2_token = token[source_token_index];
  // TODO: Add more validation
  if (current_float2_token != '(') {
    ska_strcpy(result.error_message,
               "Didn't find '(' where expected for vec default value!");
    return result;
    }

    // Now parse the first vector value
    char float2_buffer[16];
    unsigned int float2_buffer_index = 0;
    current_float2_token = token[++source_token_index];
    bool has_more_than_one_value = false;
    while (current_float2_token != ')') {
        if (current_float2_token == ',') {
            has_more_than_one_value = true;
            break;
        }
        float2_buffer[float2_buffer_index++] = current_float2_token;
        current_float2_token = token[++source_token_index];
    }
    float2_buffer[float2_buffer_index] = '\0';
    result.vector.x = strtof(float2_buffer, NULL);

    // Early out if one value
    if (!has_more_than_one_value) {
        result.vector.y = result.vector.x;
        result.vector.z = result.vector.x;
        result.vector.w = result.vector.x;
        return result;
    }

    // Now parse the rest of the values
    float* vector_values[3] = { &result.vector.y, &result.vector.z, &result.vector.w };
    unsigned int vector_values_index = 0;
    float2_buffer_index = 0;
    current_float2_token = token[++source_token_index];
    // Probably should limit this and have a better termination criterion for loop
    while (true) {
        if (current_float2_token == ',' || current_float2_token == ')') {
            float2_buffer[float2_buffer_index] = '\0';
            *vector_values[vector_values_index++] = strtof(float2_buffer, NULL);
            if (current_float2_token == ')') {
                break;
            }
            float2_buffer_index = 0;
            current_float2_token = token[++source_token_index];
        }
        float2_buffer[float2_buffer_index++] = current_float2_token;
        current_float2_token = token[++source_token_index];
    }

    return result;
}

bool shader_file_is_function_return_type_token(const char* token) {
    return strcmp(token, "void") == 0 || strcmp(token, "bool") == 0 || strcmp(token, "int") == 0
           || strcmp(token, "float") == 0 || strcmp(token, "vec2") == 0 || strcmp(token, "vec3") == 0
           || strcmp(token, "vec4") == 0;
}

typedef struct SEShaderFileParseBaseText {
    const char* vertex;
    const char* fragment;
} SEShaderFileParseBaseText;

SEShaderFileParseBaseText shader_file_get_base_shader_text(SkaShaderInstanceType shader_type) {
    switch (shader_type) {
    case SkaShaderInstanceType_SCREEN: {
        return (SEShaderFileParseBaseText) {
            .vertex = ska_opengl_shader_source_vertex_screen,
            .fragment = ska_opengl_shader_source_fragment_screen
        };
    }
    case SkaShaderInstanceType_SPRITE: {
        return (SEShaderFileParseBaseText) {
            .vertex = ska_opengl_shader_source_vertex_sprite,
            .fragment = ska_opengl_shader_source_fragment_sprite
        };
    }
    default:
        break;
    }
    return (SEShaderFileParseBaseText) {
        .vertex = NULL, .fragment = NULL
    };
}

// TODO: Check to make sure memory is cleaned up on errors
SkaShaderFileParseResult ska_shader_file_parser_parse_shader(const char* shader_source) {
  SkaShaderFileParseResult result = {.error_message = {0}};
  char *original_source = ska_strdup(shader_source);
  char *current_source = original_source;
  char shader_token[32];
  bool is_semicolon_found;
  result.parse_data =
      (SkaShaderFileParseData){.shader_type = SkaShaderInstanceType_INVALID,
                               .full_vertex_source = NULL,
                               .full_fragment_source = NULL,
                               .vertex_function_source = NULL,
                               .fragment_function_source = NULL,
                               .uniform_count = 0,
                               .function_count = 0};

  // Parse shader type
  shader_file_find_next_token(&current_source, shader_token,
                              &is_semicolon_found);
  if (strcmp(shader_token, "shader_type") != 0) {
    SHADER_FILE_PARSER_ERROR_FMT_RETURN(
        result, original_source,
        "Didn't find 'shader_type' first line!  Found '%s'", shader_token);
    }
    // Parse shader type value
    shader_file_find_next_token(&current_source, shader_token, &is_semicolon_found);
    if (strcmp(shader_token, "screen") == 0) {
      result.parse_data.shader_type = SkaShaderInstanceType_SCREEN;
    } else if (strcmp(shader_token, "sprite") == 0) {
      result.parse_data.shader_type = SkaShaderInstanceType_SPRITE;
    } else {
        SHADER_FILE_PARSER_ERROR_FMT_RETURN(result, original_source, "Didn't find 'shader_type' value on first line, instead found '%s'!", shader_token);
    }

    // Parse the rest
    while (shader_file_find_next_token(&current_source, shader_token, &is_semicolon_found)) {
        // UNIFORM VARIABLE
        if (strcmp(shader_token, "uniform") == 0) {
//            printf("Parsing shader uniform variable\n");
            SkaShaderParam shader_uniform;
            // Parse uniform type
            char shader_uniform_type_name[32];
            shader_file_find_next_token(&current_source, shader_uniform_type_name, &is_semicolon_found);
            if (strcmp(shader_uniform_type_name, "bool") == 0) {
                shader_uniform.type = SkaShaderParamType_BOOL;
                shader_uniform.value.bool_value = false;
            } else if (strcmp(shader_uniform_type_name, "int") == 0) {
                shader_uniform.type = SkaShaderParamType_INT;
                shader_uniform.value.int_value = 0;
            } else if (strcmp(shader_uniform_type_name, "float") == 0) {
                shader_uniform.type = SkaShaderParamType_FLOAT;
                shader_uniform.value.float_value = 0.0f;
            } else if (strcmp(shader_uniform_type_name, "vec2") == 0) {
                shader_uniform.type = SkaShaderParamType_FLOAT2;
                shader_uniform.value.float2_value = (SkaVector2) {
                    0.0f, 0.0f
                };
            } else if (strcmp(shader_uniform_type_name, "vec3") == 0) {
                shader_uniform.type = SkaShaderParamType_FLOAT3;
                shader_uniform.value.float3_value = (SkaVector3) {
                    0.0f, 0.0f, 0.0f
                };
            } else if (strcmp(shader_uniform_type_name, "vec4") == 0) {
                shader_uniform.type = SkaShaderParamType_FLOAT4;
                shader_uniform.value.float4_value = (SkaVector4) {
                    0.0f, 0.0f, 0.0f, 0.0f
                };
            } else {
                SHADER_FILE_PARSER_ERROR_FMT_RETURN(result, original_source, "Expected to find uniform shader type, instead found '%s'!", shader_token);
            }
//            printf("type = '%s'\n", shaderUniformTypeName);
            // Parse uniform name
            char shader_uniform_name[32];
            shader_file_find_next_token(&current_source, shader_uniform_name, &is_semicolon_found);
            shader_uniform.name = ska_strdup(shader_uniform_name);
//            printf("name = '%s'\n", shaderUniformName);
            // If we didn't find the semicolon, parse for default value
            if (!is_semicolon_found) {
//                printf("Parsing shader default value\n");
                // Look for '='
                if (!shader_file_find_next_uniform_equals_token(&current_source)) {
                    SHADER_FILE_PARSER_ERROR_FMT_RETURN(result, original_source, "Expected to find an '=' when declaring a uniform variable, instead found '%s'!", shader_token);
                }
//                printf("Found '='!\n");
                // Now parse default value
                char shader_uniform_default_value[32];
                shader_file_find_next_uniform_default_value(&current_source, shader_uniform_default_value, &is_semicolon_found);
                if (!is_semicolon_found) {
                    SHADER_FILE_PARSER_ERROR_FMT_RETURN(result, original_source, "Expected to find a ';' after declaring a uniform variable's default value, found '%s'!", shader_token);
                }
//                printf("shader uniform default value = '%s'\n", shaderUniformDefaultValue);
                switch (shader_uniform.type) {
                case SkaShaderParamType_BOOL: {
                    // xor to set to false if default value is either 'false' or '0'
                    shader_uniform.value.bool_value = !(strcmp(shader_uniform_default_value, "false") == 0 || strcmp(shader_uniform_default_value, "0") == 0);
                    break;
                }
                case SkaShaderParamType_INT: {
                    char* endptr = NULL;
                    shader_uniform.value.int_value = (int) strtol(shader_uniform_default_value, &endptr, 10);
                    if (*endptr != '\0') {
                        SHADER_FILE_PARSER_ERROR_FMT_RETURN(result, original_source, "Not a valid uniform int default value, found '%s'!", shader_token);
                    }
                    break;
                }
                case SkaShaderParamType_FLOAT: {
                    shader_uniform.value.float_value = strtof(shader_uniform_default_value, NULL);
                    break;
                }
                case SkaShaderParamType_FLOAT2: {
                    const ShaderFileParseVecParseResult vector_result = shader_file_parse_vec_default_value_token(shader_uniform_default_value);
                    shader_uniform.value.float2_value.x = vector_result.vector.x;
                    shader_uniform.value.float2_value.y = vector_result.vector.y;
                    break;
                }
                case SkaShaderParamType_FLOAT3: {
                    const ShaderFileParseVecParseResult vector_result = shader_file_parse_vec_default_value_token(shader_uniform_default_value);
                    shader_uniform.value.float3_value.x = vector_result.vector.x;
                    shader_uniform.value.float3_value.y = vector_result.vector.y;
                    shader_uniform.value.float3_value.z = vector_result.vector.z;
                    break;
                }
                case SkaShaderParamType_FLOAT4: {
                    const ShaderFileParseVecParseResult vector_result = shader_file_parse_vec_default_value_token(shader_uniform_default_value);
                    shader_uniform.value.float4_value.x = vector_result.vector.x;
                    shader_uniform.value.float4_value.y = vector_result.vector.y;
                    shader_uniform.value.float4_value.z = vector_result.vector.z;
                    shader_uniform.value.float4_value.w = vector_result.vector.w;
                    break;
                }
                default:
                    break;
                }
            }
            // Finally after all validation checks, add new uniform to array
            result.parse_data.uniforms[result.parse_data.uniform_count++] =
                shader_uniform;
        } else if (shader_file_is_function_return_type_token(shader_token)) {
            SkaShaderFileParserFunction parsed_function = shader_file_find_next_function(&current_source, shader_token);
            if (parsed_function.name == NULL ||
                parsed_function.full_function_source == NULL) {
              SHADER_FILE_PARSER_ERROR_RETURN(
                  result, original_source,
                  "Didn't successfully parse shader function!");
            }
//            printf("function name = '%s'\n", parsedFunction.name);
//            printf("function source = '%s'\n", parsedFunction.fullFunctionSource);
            // Check for vertex and fragment shader functions
            if (strcmp(parsed_function.name, "vertex") == 0) {
                result.parse_data.vertex_function_source = shader_file_parse_function_body(parsed_function.full_function_source);
                SKA_FREE(parsed_function.name);
                SKA_FREE(parsed_function.full_function_source);
            } else if (strcmp(parsed_function.name, "fragment") == 0) {
                result.parse_data.fragment_function_source = shader_file_parse_function_body(parsed_function.full_function_source);
                SKA_FREE(parsed_function.name);
                SKA_FREE(parsed_function.full_function_source);
            } else {
                // Add non vertex and fragment functions to our array
                result.parse_data.functions[result.parse_data.function_count++] = parsed_function;
            }
        } else {
            SHADER_FILE_PARSER_ERROR_FMT_RETURN(result, original_source, "Unexpected token '%s'!", shader_token);
        }
    }

    // Now that we've parsed everything create vertex and fragment source text
    const char* shader_uniforms_replace_token = "//@@UNIFORMS\n";
    const char* shader_functions_replace_token = "//@@FUNCTIONS\n";
    const char* shader_vertex_body_replace_token = "//@@vertex()\n";
    const char* shader_fragment_body_replace_token = "//@@fragment()\n";
    const usize shader_uniforms_replace_token_length = strlen(shader_uniforms_replace_token);
    const usize shader_functions_replace_token_length = strlen(shader_functions_replace_token);
    const usize shader_vertex_body_replace_token_length = strlen(shader_vertex_body_replace_token);
    const usize shader_fragment_body_replace_token_length = strlen(shader_fragment_body_replace_token);

    const SEShaderFileParseBaseText shader_base_text = shader_file_get_base_shader_text(result.parse_data.shader_type);
    char full_shader_buffer[4096];
    // Create vertex source
    ska_strcpy(full_shader_buffer, shader_base_text.vertex);
    if (result.parse_data.vertex_function_source) {
        // Vertex uniforms
        char* found_uniforms_token = strstr(full_shader_buffer, shader_uniforms_replace_token);
        if (!found_uniforms_token) {
            SHADER_FILE_PARSER_ERROR_RETURN(result, original_source, "Unable to find uniforms token in vertex shader!");
        }
        char* uniforms_source = shader_file_parse_data_get_full_uniforms_source(&result.parse_data);
        if (uniforms_source) {
            const usize uniforms_replace_length = strlen(uniforms_source);
            memmove(found_uniforms_token + uniforms_replace_length,
                    found_uniforms_token + shader_uniforms_replace_token_length,
                    strlen(found_uniforms_token + shader_uniforms_replace_token_length) + 1);
            memcpy(found_uniforms_token, uniforms_source, uniforms_replace_length);
            SKA_FREE(uniforms_source);
        }
        // Vertex functions
        char* found_functions_token = strstr(full_shader_buffer, shader_functions_replace_token);
        if (!found_functions_token) {
            SHADER_FILE_PARSER_ERROR_RETURN(result, original_source, "Unable to find functions token in vertex shader!");
        }
        char* functions_source = shader_file_parse_data_get_full_functions_source(&result.parse_data);
        if (functions_source) {
            const usize functions_replace_length = strlen(functions_source);
            memmove(found_functions_token + functions_replace_length,
                    found_functions_token + shader_functions_replace_token_length,
                    strlen(found_functions_token + shader_functions_replace_token_length) + 1);
            memcpy(found_functions_token, functions_source, functions_replace_length);
            SKA_FREE(functions_source);
        }
        // Vertex body
        char* found_vertex_token = strstr(full_shader_buffer, shader_vertex_body_replace_token);
        if (!found_vertex_token) {
            SHADER_FILE_PARSER_ERROR_RETURN(result, original_source, "Unable to find vertex() token in vertex shader!")
        }
        const usize vertex_body_replace_length = strlen(result.parse_data.vertex_function_source);
        memmove(found_vertex_token + vertex_body_replace_length,
                found_vertex_token + shader_vertex_body_replace_token_length,
                strlen(found_vertex_token + shader_vertex_body_replace_token_length) + 1);
        memcpy(found_vertex_token, result.parse_data.vertex_function_source, vertex_body_replace_length);
    }
//        printf("FULL VERTEX SOURCE = \n%s\n", fullShaderBuffer);
    result.parse_data.full_vertex_source = ska_strdup(full_shader_buffer);

    // Create fragment source
    ska_strcpy(full_shader_buffer, shader_base_text.fragment);
    if (result.parse_data.fragment_function_source) {
        // Fragment uniforms
        char* found_uniforms_token = strstr(full_shader_buffer, shader_uniforms_replace_token);
        if (!found_uniforms_token) {
            SHADER_FILE_PARSER_ERROR_RETURN(result, original_source, "Unable to find uniforms token in fragment shader!")
        }
        char* uniforms_source = shader_file_parse_data_get_full_uniforms_source(&result.parse_data);
        if (uniforms_source) {
            const usize uniforms_replace_length = strlen(uniforms_source);
            memmove(found_uniforms_token + uniforms_replace_length,
                    found_uniforms_token + shader_uniforms_replace_token_length,
                    strlen(found_uniforms_token + shader_uniforms_replace_token_length) + 1);
            memcpy(found_uniforms_token, uniforms_source, uniforms_replace_length);
            SKA_FREE(uniforms_source);
        }
        // Fragment functions
        char* found_functions_token = strstr(full_shader_buffer, shader_functions_replace_token);
        if (!found_functions_token) {
            SHADER_FILE_PARSER_ERROR_RETURN(result, original_source, "Unable to find functions token in fragment shader!")
        }
        char* functions_source = shader_file_parse_data_get_full_functions_source(&result.parse_data);
        if (functions_source) {
            const usize functions_replace_length = strlen(functions_source);
            memmove(found_functions_token + functions_replace_length,
                    found_functions_token + shader_functions_replace_token_length,
                    strlen(found_functions_token + shader_functions_replace_token_length) + 1);
            memcpy(found_functions_token, functions_source, functions_replace_length);
            SKA_FREE(functions_source);
        }
        // Fragment body
        char* found_fragment_token = strstr(full_shader_buffer, shader_fragment_body_replace_token);
        if (!found_fragment_token) {
            SHADER_FILE_PARSER_ERROR_RETURN(result, original_source, "Unable to find fragment() token in fragment shader!")
        }
        const usize fragment_body_replace_length = strlen(result.parse_data.fragment_function_source);
        memmove(found_fragment_token + fragment_body_replace_length,
                found_fragment_token + shader_fragment_body_replace_token_length,
                strlen(found_fragment_token + shader_fragment_body_replace_token_length) + 1);
        memcpy(found_fragment_token, result.parse_data.fragment_function_source, fragment_body_replace_length);
    }
//        printf("FULL FRAGMENT SOURCE = \n%s\n", fullShaderBuffer);
    result.parse_data.full_fragment_source = ska_strdup(full_shader_buffer);

    SKA_FREE(original_source);

    return result;
}

void ska_shader_file_parse_clear_parse_result(SkaShaderFileParseResult* result) {
    if (result->parse_data.fragment_function_source) {
        SKA_FREE(result->parse_data.fragment_function_source);
    }
    if (result->parse_data.vertex_function_source) {
        SKA_FREE(result->parse_data.vertex_function_source);
    }
    if (result->parse_data.full_vertex_source) {
        SKA_FREE(result->parse_data.full_vertex_source);
    }
    if (result->parse_data.full_fragment_source) {
        SKA_FREE(result->parse_data.full_fragment_source);
    }
    for (usize i = 0; i < result->parse_data.uniform_count; i++) {
        SKA_FREE(result->parse_data.uniforms[i].name);
    }
    for (usize i = 0; i < result->parse_data.function_count; i++) {
        SKA_FREE(result->parse_data.functions[i].name);
        SKA_FREE(result->parse_data.functions[i].full_function_source);
    }
}

#endif // #if SKA_RENDERING
