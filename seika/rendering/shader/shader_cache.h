#pragma once

#include "shader_instance_minimal.h"
#include "shader_instance.h"

// Holds references to all custom shaders

void ska_shader_cache_initialize();
void ska_shader_cache_finalize();

SkaShaderInstanceId ska_shader_cache_add_instance(SkaShaderInstance* instance);
void ska_shader_cache_remove_instance(SkaShaderInstanceId instanceId);
SkaShaderInstance* ska_shader_cache_get_instance(SkaShaderInstanceId instanceId);
SkaShaderInstance* ska_shader_cache_get_instance_checked(SkaShaderInstanceId instanceId);
// Expects .shader files which contain vertex and fragment implementations
SkaShaderInstanceId ska_shader_cache_create_instance_and_add(const char* shaderPath);
// Should be a modification to the base shaders found in 'shader_source.h'
SkaShaderInstanceId ska_shader_cache_create_instance_and_add_from_raw(const char* vertexPath, const char* fragmentPath);
SkaShaderInstanceId ska_shader_cache_create_instance_and_add_from_source(const char* vertexSource, const char* fragmentSource);
