#pragma once

#if SKA_RENDERING

#include "shader_instance.h"

typedef struct SkaShaderFileParserFunction {
    char* name;
    char *full_function_source;
} SkaShaderFileParserFunction;

typedef struct SkaShaderFileParseData {
  SkaShaderInstanceType shader_type;
  char *full_vertex_source;
  char *full_fragment_source;
  char *vertex_function_source;
  char *fragment_function_source;
  usize uniform_count;
  usize function_count;
  SkaShaderParam uniforms[32];
  SkaShaderFileParserFunction functions[32];
} SkaShaderFileParseData;

typedef struct SkaShaderFileParseResult {
  char error_message[128];
  SkaShaderFileParseData parse_data;
} SkaShaderFileParseResult;

SkaShaderFileParseResult
ska_shader_file_parser_parse_shader(const char *shader_source);
void ska_shader_file_parse_clear_parse_result(SkaShaderFileParseResult* result);

#endif // #if SKA_RENDERING
