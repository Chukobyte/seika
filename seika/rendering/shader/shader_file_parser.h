#pragma once

#include "shader_instance.h"

typedef struct SkaShaderFileParserFunction {
    char* name;
    char* fullFunctionSource;
} SkaShaderFileParserFunction;

typedef struct SkaShaderFileParseData {
    SkaShaderInstanceType shaderType;
    char* fullVertexSource;
    char* fullFragmentSource;
    char* vertexFunctionSource;
    char* fragmentFunctionSource;
    usize uniformCount;
    usize functionCount;
    SkaShaderParam uniforms[32];
    SkaShaderFileParserFunction functions[32];
} SkaShaderFileParseData;

typedef struct SkaShaderFileParseResult {
    char errorMessage[128];
    SkaShaderFileParseData  parseData;
} SkaShaderFileParseResult;

SkaShaderFileParseResult ska_shader_file_parser_parse_shader(const char* shaderSource);
void ska_shader_file_parse_clear_parse_result(SkaShaderFileParseResult* result);
