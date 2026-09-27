#pragma once

#if SKA_RENDERING

#include "shader_instance_minimal.h"
#include "shader_instance.h"

// Holds references to all custom shaders

void ska_shader_cache_initialize();
void ska_shader_cache_finalize();

SkaShaderInstanceId ska_shader_cache_add_instance(SkaShaderInstance* instance);
void ska_shader_cache_remove_instance(SkaShaderInstanceId instance_id);
SkaShaderInstance* ska_shader_cache_get_instance(SkaShaderInstanceId instance_id);
SkaShaderInstance* ska_shader_cache_get_instance_checked(SkaShaderInstanceId instance_id);
// Expects .shader files which contain vertex and fragment implementations
SkaShaderInstanceId ska_shader_cache_create_instance_and_add(const char* shader_path);
// Should be a modification to the base shaders found in 'shader_source.h'
SkaShaderInstanceId ska_shader_cache_create_instance_and_add_from_raw(const char* vertex_path, const char* fragment_path);
SkaShaderInstanceId ska_shader_cache_create_instance_and_add_from_source(const char* vertex_source, const char* fragment_source);

#endif // #if SKA_RENDERING
