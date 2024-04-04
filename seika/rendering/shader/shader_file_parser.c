#include "shader_file_parser.h"

#include <string.h>

#include "shader_source.h"
#include "seika/string.h"
#include "seika/memory.h"

#define SHADER_FILE_PARSER_ERROR_RETURN(RESULT, SOURCE, MESSAGE) \
ska_strcpy((RESULT).errorMessage, (MESSAGE));                        \
SKA_MEM_FREE((SOURCE));                                           \
return (RESULT);

#define SHADER_FILE_PARSER_ERROR_FMT_RETURN(RESULT, SOURCE, FMT, ...) \
sprintf((RESULT).errorMessage, (FMT), ##__VA_ARGS__);                 \
SKA_MEM_FREE((SOURCE));                                                \
return (RESULT);

static char* shader_file_parse_data_get_full_uniforms_source(SkaShaderFileParseData* parseData) {
    if (parseData->uniformCount == 0) {
        return NULL;
    }
    char uniformsBuffer[1024];
    uniformsBuffer[0] = '\0';
    for (usize i = 0; i < parseData->uniformCount; i++) {
        ska_strcat(uniformsBuffer, "uniform ");
        switch (parseData->uniforms[i].type) {
        case SkaShaderParamType_BOOL: {
            ska_strcat(uniformsBuffer, "bool ");
            break;
        }
        case SkaShaderParamType_INT: {
            ska_strcat(uniformsBuffer, "int ");
            break;
        }
        case SkaShaderParamType_FLOAT: {
            ska_strcat(uniformsBuffer, "float ");
            break;
        }
        case SkaShaderParamType_FLOAT2: {
            ska_strcat(uniformsBuffer, "vec2 ");
            break;
        }
        case SkaShaderParamType_FLOAT3: {
            ska_strcat(uniformsBuffer, "vec3 ");
            break;
        }
        case SkaShaderParamType_FLOAT4: {
            ska_strcat(uniformsBuffer, "vec4 ");
            break;
        }
        default:
            break;
        }
        ska_strcat(uniformsBuffer, parseData->uniforms[i].name);
        ska_strcat(uniformsBuffer, ";\n");
    }
    return ska_strdup(uniformsBuffer);
}

char* shader_file_parse_data_get_full_functions_source(SkaShaderFileParseData* parseData) {
    if (parseData->functionCount == 0) {
        return NULL;
    }
    char functionsBuffer[4096];
    functionsBuffer[0] = '\0';
    for (usize i = 0; i < parseData->functionCount; i++) {
        ska_strcat(functionsBuffer, parseData->functions[i].fullFunctionSource);
        ska_strcat(functionsBuffer, "\n");
    }
    return ska_strdup(functionsBuffer);
}

void shader_file_parse_data_delete_internal_memory(SkaShaderFileParseData* parseData) {
    SKA_MEM_FREE(parseData->fullVertexSource);
    SKA_MEM_FREE(parseData->fullFragmentSource);
    for (usize i = 0; i < parseData->functionCount; i++) {
        SKA_MEM_FREE(parseData->functions[i].name);
        SKA_MEM_FREE(parseData->functions[i].fullFunctionSource);
    }
}

// Will parse the next token, returns true if there is more source to parse
bool shader_file_find_next_token(char** shaderSource, char* tokenOut, bool* semiColonFound) {
    tokenOut[0] = '\0';

    if (semiColonFound) {
        *semiColonFound = false;
    }

    int tokenIndex = 0;
    while (*(*shaderSource) != '\0') {
        if (*(*shaderSource) == ' ' || *(*shaderSource) == ';') {
            if (semiColonFound && *(*shaderSource) == ';') {
                *semiColonFound = true;
            }
            (*shaderSource)++;
            tokenOut[tokenIndex] = '\0';
            return true;
        }
        if (*(*shaderSource) != '\n') {
            tokenOut[tokenIndex++] = *(*shaderSource);
        }
        (*shaderSource)++;
    }
    return false;
}

// Continue parsing shader source and returns 'true' if an '=' was found
bool shader_file_find_next_uniform_equals_token(char** shaderSource) {
    while (*(*shaderSource) != '\0') {
        if (*(*shaderSource) == '=') {
            (*shaderSource)++;
            return true;
        }
        (*shaderSource)++;
    }
    return false;
}

bool shader_file_find_next_uniform_default_value(char** shaderSource, char* tokenOut, bool* semiColonFound) {
    tokenOut[0] = '\0';

    if (semiColonFound) {
        *semiColonFound = false;
    }

    int tokenIndex = 0;
    while (*(*shaderSource) != '\0') {
        if (*(*shaderSource) == ';') {
            if (semiColonFound) {
                *semiColonFound = true;
            }
            (*shaderSource)++;
            tokenOut[tokenIndex] = '\0';
            return true;
        }
        if (*(*shaderSource) != '\n' && *(*shaderSource) != ' ') {
            tokenOut[tokenIndex++] = *(*shaderSource);
        }
        (*shaderSource)++;
    }
    return false;
}

SkaShaderFileParserFunction shader_file_find_next_function(char** shaderSource, const char* functionReturnType) {
    SkaShaderFileParserFunction parsedFunction = { .name = NULL, .fullFunctionSource = NULL };
    char shaderFunctionBuffer[1024];
    ska_strcpy(shaderFunctionBuffer, functionReturnType);
    ska_strcat(shaderFunctionBuffer, " ");
    usize bufferIndex = strlen(shaderFunctionBuffer);

    // Get name first
    char shaderFunctionName[64];
    unsigned int shaderFunctionNameIndex = 0;
    while (*(*shaderSource) != '\0') {
        shaderFunctionBuffer[bufferIndex++] = *(*shaderSource);
        if (*(*shaderSource) != ' ') {
            shaderFunctionName[shaderFunctionNameIndex++] = *(*shaderSource);
        }
        (*shaderSource)++;
        if (*(*shaderSource) == '(') {
            shaderFunctionName[shaderFunctionNameIndex] = '\0';
            break;
        }
    }
    // Now just loop through until we encounter a '}'
    usize openBracketsCount = 0;
    while (*(*shaderSource) != '\0') {
        shaderFunctionBuffer[bufferIndex++] = *(*shaderSource);
        (*shaderSource)++;
        if (*(*shaderSource) == '{') {
            openBracketsCount++;
        } else if (*(*shaderSource) == '}') {
            shaderFunctionBuffer[bufferIndex++] = *(*shaderSource);
            (*shaderSource)++;
            openBracketsCount--;
            if (openBracketsCount == 0) {
                shaderFunctionBuffer[bufferIndex] = '\0';
                break;
            }
        }
    }

    parsedFunction.name = ska_strdup(shaderFunctionName);
    parsedFunction.fullFunctionSource = ska_strdup(shaderFunctionBuffer);
    return parsedFunction;
}

// Should have a valid function at this point...
char* shader_file_parse_function_body(const char* functionSource) {
    char shaderFunctionBuffer[1024];
    shaderFunctionBuffer[0] = '\0';
    ska_strcpy(shaderFunctionBuffer, functionSource);
    unsigned int functionBufferIndex = 0;
    char currentToken = shaderFunctionBuffer[functionBufferIndex];
    // Find beginning body
    while (currentToken != '{') {
        currentToken = shaderFunctionBuffer[++functionBufferIndex];
    }
    char functionBodyBuffer[1024];
    unsigned int functionBodyIndex = 0;
    currentToken = shaderFunctionBuffer[++functionBufferIndex];
    // Return NULL if an empty function with (with no newlines at least)
    if (currentToken == '}') {
        return NULL;
    }
    usize openBracketsCount = 1;
    while (openBracketsCount > 0) {
        currentToken = shaderFunctionBuffer[functionBufferIndex++];
        functionBodyBuffer[functionBodyIndex++] = currentToken;
        if (currentToken == '{') {
            openBracketsCount++;
        } else if (currentToken == '}') {
            openBracketsCount--;
        }
    }
    functionBodyBuffer[functionBodyIndex - 1] = '\n';
    functionBodyBuffer[functionBodyIndex] = '\0';

    return ska_strdup(functionBodyBuffer);
}

typedef struct ShaderFileParseVecParseResult {
    SkaVector4 vector;
    char errorMessage[64];
} ShaderFileParseVecParseResult;

// TODO: Refactor and make shorter...
ShaderFileParseVecParseResult shader_file_parse_vec_default_value_token(const char* token) {
    ShaderFileParseVecParseResult result = { .vector = { .x = 0.0f, .y = 0.0f, .z = 0.0f, .w = 0.0f },
                                             .errorMessage = {0}
                                           };
    unsigned int sourceTokenIndex = 4;
    char currentFloat2Token = token[sourceTokenIndex];
    // TODO: Add more validation
    if (currentFloat2Token != '(') {
        ska_strcpy(result.errorMessage, "Didn't find '(' where expected for vec default value!");
        return result;
    }

    // Now parse the first vector value
    char float2Buffer[16];
    unsigned int float2BufferIndex = 0;
    currentFloat2Token = token[++sourceTokenIndex];
    bool hasMoreThanOneValue = false;
    while (currentFloat2Token != ')') {
        if (currentFloat2Token == ',') {
            hasMoreThanOneValue = true;
            break;
        }
        float2Buffer[float2BufferIndex++] = currentFloat2Token;
        currentFloat2Token = token[++sourceTokenIndex];
    }
    float2Buffer[float2BufferIndex] = '\0';
    result.vector.x = strtof(float2Buffer, NULL);

    // Early out if one value
    if (!hasMoreThanOneValue) {
        result.vector.y = result.vector.x;
        result.vector.z = result.vector.x;
        result.vector.w = result.vector.x;
        return result;
    }

    // Now parse the rest of the values
    float* vectorValues[3] = { &result.vector.y, &result.vector.z, &result.vector.w };
    unsigned int vectorValuesIndex = 0;
    float2BufferIndex = 0;
    currentFloat2Token = token[++sourceTokenIndex];
    // Probably should limit this and have a better termination criterion for loop
    while (true) {
        if (currentFloat2Token == ',' || currentFloat2Token == ')') {
            float2Buffer[float2BufferIndex] = '\0';
            *vectorValues[vectorValuesIndex++] = strtof(float2Buffer, NULL);
            if (currentFloat2Token == ')') {
                break;
            }
            float2BufferIndex = 0;
            currentFloat2Token = token[++sourceTokenIndex];
        }
        float2Buffer[float2BufferIndex++] = currentFloat2Token;
        currentFloat2Token = token[++sourceTokenIndex];
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

SEShaderFileParseBaseText shader_file_get_base_shader_text(SkaShaderInstanceType shaderType) {
    switch (shaderType) {
    case SkaShaderInstanceType_SCREEN: {
        return (SEShaderFileParseBaseText) {
            .vertex = SKA_OPENGL_SHADER_SOURCE_VERTEX_SCREEN,
            .fragment = SKA_OPENGL_SHADER_SOURCE_FRAGMENT_SCREEN
        };
    }
    case SkaShaderInstanceType_SPRITE: {
        return (SEShaderFileParseBaseText) {
            .vertex = SKA_OPENGL_SHADER_SOURCE_VERTEX_SPRITE,
            .fragment = SKA_OPENGL_SHADER_SOURCE_FRAGMENT_SPRITE
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
SkaShaderFileParseResult ska_shader_file_parser_parse_shader(const char* shaderSource) {
    SkaShaderFileParseResult result = {.errorMessage = {0}};
    char *originalSource = ska_strdup(shaderSource);
    char *currentSource = originalSource;
    char shaderToken[32];
    bool isSemicolonFound;
    result.parseData = (SkaShaderFileParseData) {
        .shaderType = SkaShaderInstanceType_INVALID,
        .fullVertexSource = NULL,
        .fullFragmentSource = NULL, .vertexFunctionSource = NULL,
        .fragmentFunctionSource = NULL,
        .uniformCount = 0,
        .functionCount = 0
    };

    // Parse shader type
    shader_file_find_next_token(&currentSource, shaderToken, &isSemicolonFound);
    if (strcmp(shaderToken, "shader_type") != 0) {
        SHADER_FILE_PARSER_ERROR_FMT_RETURN(result, originalSource, "Didn't find 'shader_type' first line!  Found '%s'", shaderToken);
    }
    // Parse shader type value
    shader_file_find_next_token(&currentSource, shaderToken, &isSemicolonFound);
    if (strcmp(shaderToken, "screen") == 0) {
        result.parseData.shaderType = SkaShaderInstanceType_SCREEN;
    } else if (strcmp(shaderToken, "sprite") == 0) {
        result.parseData.shaderType = SkaShaderInstanceType_SPRITE;
    } else {
        SHADER_FILE_PARSER_ERROR_FMT_RETURN(result, originalSource, "Didn't find 'shader_type' value on first line, instead found '%s'!", shaderToken);
    }

    // Parse the rest
    while (shader_file_find_next_token(&currentSource, shaderToken, &isSemicolonFound)) {
        // UNIFORM VARIABLE
        if (strcmp(shaderToken, "uniform") == 0) {
//            printf("Parsing shader uniform variable\n");
            SkaShaderParam shaderUniform;
            // Parse uniform type
            char shaderUniformTypeName[32];
            shader_file_find_next_token(&currentSource, shaderUniformTypeName, &isSemicolonFound);
            if (strcmp(shaderUniformTypeName, "bool") == 0) {
                shaderUniform.type = SkaShaderParamType_BOOL;
                shaderUniform.value.boolValue = false;
            } else if (strcmp(shaderUniformTypeName, "int") == 0) {
                shaderUniform.type = SkaShaderParamType_INT;
                shaderUniform.value.intValue = 0;
            } else if (strcmp(shaderUniformTypeName, "float") == 0) {
                shaderUniform.type = SkaShaderParamType_FLOAT;
                shaderUniform.value.floatValue = 0.0f;
            } else if (strcmp(shaderUniformTypeName, "vec2") == 0) {
                shaderUniform.type = SkaShaderParamType_FLOAT2;
                shaderUniform.value.float2Value = (SkaVector2) {
                    0.0f, 0.0f
                };
            } else if (strcmp(shaderUniformTypeName, "vec3") == 0) {
                shaderUniform.type = SkaShaderParamType_FLOAT3;
                shaderUniform.value.float3Value = (SkaVector3) {
                    0.0f, 0.0f, 0.0f
                };
            } else if (strcmp(shaderUniformTypeName, "vec4") == 0) {
                shaderUniform.type = SkaShaderParamType_FLOAT4;
                shaderUniform.value.float4Value = (SkaVector4) {
                    0.0f, 0.0f, 0.0f, 0.0f
                };
            } else {
                SHADER_FILE_PARSER_ERROR_FMT_RETURN(result, originalSource, "Expected to find uniform shader type, instead found '%s'!", shaderToken);
            }
//            printf("type = '%s'\n", shaderUniformTypeName);
            // Parse uniform name
            char shaderUniformName[32];
            shader_file_find_next_token(&currentSource, shaderUniformName, &isSemicolonFound);
            shaderUniform.name = ska_strdup(shaderUniformName);
//            printf("name = '%s'\n", shaderUniformName);
            // If we didn't find the semicolon, parse for default value
            if (!isSemicolonFound) {
//                printf("Parsing shader default value\n");
                // Look for '='
                if (!shader_file_find_next_uniform_equals_token(&currentSource)) {
                    SHADER_FILE_PARSER_ERROR_FMT_RETURN(result, originalSource, "Expected to find an '=' when declaring a uniform variable, instead found '%s'!", shaderToken);
                }
//                printf("Found '='!\n");
                // Now parse default value
                char shaderUniformDefaultValue[32];
                shader_file_find_next_uniform_default_value(&currentSource, shaderUniformDefaultValue, &isSemicolonFound);
                if (!isSemicolonFound) {
                    SHADER_FILE_PARSER_ERROR_FMT_RETURN(result, originalSource, "Expected to find a ';' after declaring a uniform variable's default value, found '%s'!", shaderToken);
                }
//                printf("shader uniform default value = '%s'\n", shaderUniformDefaultValue);
                switch (shaderUniform.type) {
                case SkaShaderParamType_BOOL: {
                    // xor to set to false if default value is either 'false' or '0'
                    shaderUniform.value.boolValue = !(strcmp(shaderUniformDefaultValue, "false") == 0 || strcmp(shaderUniformDefaultValue, "0") == 0);
                    break;
                }
                case SkaShaderParamType_INT: {
                    char* endptr = NULL;
                    shaderUniform.value.intValue = (int) strtol(shaderUniformDefaultValue, &endptr, 10);
                    if (*endptr != '\0') {
                        SHADER_FILE_PARSER_ERROR_FMT_RETURN(result, originalSource, "Not a valid uniform int default value, found '%s'!", shaderToken);
                    }
                    break;
                }
                case SkaShaderParamType_FLOAT: {
                    shaderUniform.value.floatValue = strtof(shaderUniformDefaultValue, NULL);
                    break;
                }
                case SkaShaderParamType_FLOAT2: {
                    const ShaderFileParseVecParseResult vectorResult = shader_file_parse_vec_default_value_token(shaderUniformDefaultValue);
                    shaderUniform.value.float2Value.x = vectorResult.vector.x;
                    shaderUniform.value.float2Value.y = vectorResult.vector.y;
                    break;
                }
                case SkaShaderParamType_FLOAT3: {
                    const ShaderFileParseVecParseResult vectorResult = shader_file_parse_vec_default_value_token(shaderUniformDefaultValue);
                    shaderUniform.value.float3Value.x = vectorResult.vector.x;
                    shaderUniform.value.float3Value.y = vectorResult.vector.y;
                    shaderUniform.value.float3Value.z = vectorResult.vector.z;
                    break;
                }
                case SkaShaderParamType_FLOAT4: {
                    const ShaderFileParseVecParseResult vectorResult = shader_file_parse_vec_default_value_token(shaderUniformDefaultValue);
                    shaderUniform.value.float4Value.x = vectorResult.vector.x;
                    shaderUniform.value.float4Value.y = vectorResult.vector.y;
                    shaderUniform.value.float4Value.z = vectorResult.vector.z;
                    shaderUniform.value.float4Value.w = vectorResult.vector.w;
                    break;
                }
                default:
                    break;
                }
            }
            // Finally after all validation checks, add new uniform to array
            result.parseData.uniforms[result.parseData.uniformCount++] = shaderUniform;
        } else if (shader_file_is_function_return_type_token(shaderToken)) {
            SkaShaderFileParserFunction parsedFunction = shader_file_find_next_function(&currentSource, shaderToken);
            if (parsedFunction.name == NULL || parsedFunction.fullFunctionSource == NULL) {
                SHADER_FILE_PARSER_ERROR_RETURN(result, originalSource, "Didn't successfully parse shader function!");
            }
//            printf("function name = '%s'\n", parsedFunction.name);
//            printf("function source = '%s'\n", parsedFunction.fullFunctionSource);
            // Check for vertex and fragment shader functions
            if (strcmp(parsedFunction.name, "vertex") == 0) {
                result.parseData.vertexFunctionSource = shader_file_parse_function_body(parsedFunction.fullFunctionSource);
                SKA_MEM_FREE(parsedFunction.name);
                SKA_MEM_FREE(parsedFunction.fullFunctionSource);
            } else if (strcmp(parsedFunction.name, "fragment") == 0) {
                result.parseData.fragmentFunctionSource = shader_file_parse_function_body(parsedFunction.fullFunctionSource);
                SKA_MEM_FREE(parsedFunction.name);
                SKA_MEM_FREE(parsedFunction.fullFunctionSource);
            } else {
                // Add non vertex and fragment functions to our array
                result.parseData.functions[result.parseData.functionCount++] = parsedFunction;
            }
        } else {
            SHADER_FILE_PARSER_ERROR_FMT_RETURN(result, originalSource, "Unexpected token '%s'!", shaderToken);
        }
    }

    // Now that we've parsed everything create vertex and fragment source text
    const char* SHADER_UNIFORMS_REPLACE_TOKEN = "//@@UNIFORMS\n";
    const char* SHADER_FUNCTIONS_REPLACE_TOKEN = "//@@FUNCTIONS\n";
    const char* SHADER_VERTEX_BODY_REPLACE_TOKEN = "//@@vertex()\n";
    const char* SHADER_FRAGMENT_BODY_REPLACE_TOKEN = "//@@fragment()\n";
    const usize SHADER_UNIFORMS_REPLACE_TOKEN_LENGTH = strlen(SHADER_UNIFORMS_REPLACE_TOKEN);
    const usize SHADER_FUNCTIONS_REPLACE_TOKEN_LENGTH = strlen(SHADER_FUNCTIONS_REPLACE_TOKEN);
    const usize SHADER_VERTEX_BODY_REPLACE_TOKEN_LENGTH = strlen(SHADER_VERTEX_BODY_REPLACE_TOKEN);
    const usize SHADER_FRAGMENT_BODY_REPLACE_TOKEN_LENGTH = strlen(SHADER_FRAGMENT_BODY_REPLACE_TOKEN);

    const SEShaderFileParseBaseText shaderBaseText = shader_file_get_base_shader_text(result.parseData.shaderType);
    char fullShaderBuffer[4096];
    // Create vertex source
    ska_strcpy(fullShaderBuffer, shaderBaseText.vertex);
    if (result.parseData.vertexFunctionSource) {
        // Vertex uniforms
        char* foundUniformsToken = strstr(fullShaderBuffer, SHADER_UNIFORMS_REPLACE_TOKEN);
        if (!foundUniformsToken) {
            SHADER_FILE_PARSER_ERROR_RETURN(result, originalSource, "Unable to find uniforms token in vertex shader!");
        }
        char* uniformsSource = shader_file_parse_data_get_full_uniforms_source(&result.parseData);
        if (uniformsSource) {
            const usize uniformsReplaceLength = strlen(uniformsSource);
            memmove(foundUniformsToken + uniformsReplaceLength,
                    foundUniformsToken + SHADER_UNIFORMS_REPLACE_TOKEN_LENGTH,
                    strlen(foundUniformsToken + SHADER_UNIFORMS_REPLACE_TOKEN_LENGTH) + 1);
            memcpy(foundUniformsToken, uniformsSource, uniformsReplaceLength);
            SKA_MEM_FREE(uniformsSource);
        }
        // Vertex functions
        char* foundFunctionsToken = strstr(fullShaderBuffer, SHADER_FUNCTIONS_REPLACE_TOKEN);
        if (!foundFunctionsToken) {
            SHADER_FILE_PARSER_ERROR_RETURN(result, originalSource, "Unable to find functions token in vertex shader!");
        }
        char* functionsSource = shader_file_parse_data_get_full_functions_source(&result.parseData);
        if (functionsSource) {
            const usize functionsReplaceLength = strlen(functionsSource);
            memmove(foundFunctionsToken + functionsReplaceLength,
                    foundFunctionsToken + SHADER_FUNCTIONS_REPLACE_TOKEN_LENGTH,
                    strlen(foundFunctionsToken + SHADER_FUNCTIONS_REPLACE_TOKEN_LENGTH) + 1);
            memcpy(foundFunctionsToken, functionsSource, functionsReplaceLength);
            SKA_MEM_FREE(functionsSource);
        }
        // Vertex body
        char* foundVertexToken = strstr(fullShaderBuffer, SHADER_VERTEX_BODY_REPLACE_TOKEN);
        if (!foundVertexToken) {
            SHADER_FILE_PARSER_ERROR_RETURN(result, originalSource, "Unable to find vertex() token in vertex shader!")
        }
        const usize vertexBodyReplaceLength = strlen(result.parseData.vertexFunctionSource);
        memmove(foundVertexToken + vertexBodyReplaceLength,
                foundVertexToken + SHADER_VERTEX_BODY_REPLACE_TOKEN_LENGTH,
                strlen(foundVertexToken + SHADER_VERTEX_BODY_REPLACE_TOKEN_LENGTH) + 1);
        memcpy(foundVertexToken, result.parseData.vertexFunctionSource, vertexBodyReplaceLength);
    }
//        printf("FULL VERTEX SOURCE = \n%s\n", fullShaderBuffer);
    result.parseData.fullVertexSource = ska_strdup(fullShaderBuffer);

    // Create fragment source
    ska_strcpy(fullShaderBuffer, shaderBaseText.fragment);
    if (result.parseData.fragmentFunctionSource) {
        // Fragment uniforms
        char* foundUniformsToken = strstr(fullShaderBuffer, SHADER_UNIFORMS_REPLACE_TOKEN);
        if (!foundUniformsToken) {
            SHADER_FILE_PARSER_ERROR_RETURN(result, originalSource, "Unable to find uniforms token in fragment shader!")
        }
        char* uniformsSource = shader_file_parse_data_get_full_uniforms_source(&result.parseData);
        if (uniformsSource) {
            const usize uniformsReplaceLength = strlen(uniformsSource);
            memmove(foundUniformsToken + uniformsReplaceLength,
                    foundUniformsToken + SHADER_UNIFORMS_REPLACE_TOKEN_LENGTH,
                    strlen(foundUniformsToken + SHADER_UNIFORMS_REPLACE_TOKEN_LENGTH) + 1);
            memcpy(foundUniformsToken, uniformsSource, uniformsReplaceLength);
            SKA_MEM_FREE(uniformsSource);
        }
        // Fragment functions
        char* foundFunctionsToken = strstr(fullShaderBuffer, SHADER_FUNCTIONS_REPLACE_TOKEN);
        if (!foundFunctionsToken) {
            SHADER_FILE_PARSER_ERROR_RETURN(result, originalSource, "Unable to find functions token in fragment shader!")
        }
        char* functionsSource = shader_file_parse_data_get_full_functions_source(&result.parseData);
        if (functionsSource) {
            const usize functionsReplaceLength = strlen(functionsSource);
            memmove(foundFunctionsToken + functionsReplaceLength,
                    foundFunctionsToken + SHADER_FUNCTIONS_REPLACE_TOKEN_LENGTH,
                    strlen(foundFunctionsToken + SHADER_FUNCTIONS_REPLACE_TOKEN_LENGTH) + 1);
            memcpy(foundFunctionsToken, functionsSource, functionsReplaceLength);
            SKA_MEM_FREE(functionsSource);
        }
        // Fragment body
        char* foundFragmentToken = strstr(fullShaderBuffer, SHADER_FRAGMENT_BODY_REPLACE_TOKEN);
        if (!foundFragmentToken) {
            SHADER_FILE_PARSER_ERROR_RETURN(result, originalSource, "Unable to find fragment() token in fragment shader!")
        }
        const usize fragmentBodyReplaceLength = strlen(result.parseData.fragmentFunctionSource);
        memmove(foundFragmentToken + fragmentBodyReplaceLength,
                foundFragmentToken + SHADER_FRAGMENT_BODY_REPLACE_TOKEN_LENGTH,
                strlen(foundFragmentToken + SHADER_FRAGMENT_BODY_REPLACE_TOKEN_LENGTH) + 1);
        memcpy(foundFragmentToken, result.parseData.fragmentFunctionSource, fragmentBodyReplaceLength);
    }
//        printf("FULL FRAGMENT SOURCE = \n%s\n", fullShaderBuffer);
    result.parseData.fullFragmentSource = ska_strdup(fullShaderBuffer);

    SKA_MEM_FREE(originalSource);

    return result;
}

void ska_shader_file_parse_clear_parse_result(SkaShaderFileParseResult* result) {
    if (result->parseData.fragmentFunctionSource) {
        SKA_MEM_FREE(result->parseData.fragmentFunctionSource);
    }
    if (result->parseData.vertexFunctionSource) {
        SKA_MEM_FREE(result->parseData.vertexFunctionSource);
    }
    if (result->parseData.fullVertexSource) {
        SKA_MEM_FREE(result->parseData.fullVertexSource);
    }
    if (result->parseData.fullFragmentSource) {
        SKA_MEM_FREE(result->parseData.fullFragmentSource);
    }
    for (usize i = 0; i < result->parseData.uniformCount; i++) {
        SKA_MEM_FREE(result->parseData.uniforms[i].name);
    }
    for (usize i = 0; i < result->parseData.functionCount; i++) {
        SKA_MEM_FREE(result->parseData.functions[i].name);
        SKA_MEM_FREE(result->parseData.functions[i].fullFunctionSource);
    }
}
