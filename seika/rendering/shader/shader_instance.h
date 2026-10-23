#pragma once

#if SKA_RENDERING

/*
 * Shader Instance
 * ---------------------------------------------------------------------------------------------------------------------
 * A work in progress api to create shader instances with parameters.
 */

#include "shader.h"
#include "seika/data_structures/hash_map_string.h"

// Shader Param - Shader instance representation of uniform parameters
typedef enum SkaShaderParamType {
    SkaShaderParamType_BOOL = 0,
    SkaShaderParamType_INT = 1,
    SkaShaderParamType_FLOAT = 2,
    SkaShaderParamType_FLOAT2 = 3,
    SkaShaderParamType_FLOAT3 = 4,
    SkaShaderParamType_FLOAT4 = 5,
} SkaShaderParamType;

typedef struct SkaShaderParam {
    char* name;
    SkaShaderParamType type;
    union {
        bool bool_value;
        int32 int_value;
        f32 float_value;
        SkaVector2 float2_value;
        SkaVector3 float3_value;
        SkaVector4 float4_value;
    } value;
} SkaShaderParam;

// Shader Instance
typedef enum SkaShaderInstanceType {
    SkaShaderInstanceType_INVALID = -1,
    SkaShaderInstanceType_SCREEN = 0,
    SkaShaderInstanceType_SPRITE = 1,
} SkaShaderInstanceType;

typedef struct SkaShaderInstance {
    SkaShader* shader;
    SkaStringHashMap* param_map;
    bool params_dirty;
} SkaShaderInstance;

SkaShaderInstance* ska_shader_instance_create(const char* vertex_source, const char* fragment_source);
SkaShaderInstance* ska_shader_instance_create_from_shader(SkaShader* shader);
void ska_shader_instance_destroy(SkaShaderInstance* shader_instance);

void ska_shader_instance_param_create_from_copy(SkaShaderInstance* shader_instance, SkaShaderParam* param);
SkaShaderParam* ska_shader_instance_param_create_bool(SkaShaderInstance* shader_instance, const char* name, bool value);
SkaShaderParam* ska_shader_instance_param_create_int(SkaShaderInstance* shader_instance, const char* name, int32 value);
SkaShaderParam* ska_shader_instance_param_create_float(SkaShaderInstance* shader_instance, const char* name, f32 value);
SkaShaderParam* ska_shader_instance_param_create_float2(SkaShaderInstance* shader_instance, const char* name, SkaVector2 value);
SkaShaderParam* ska_shader_instance_param_create_float3(SkaShaderInstance* shader_instance, const char* name, SkaVector3 value);
SkaShaderParam* ska_shader_instance_param_create_float4(SkaShaderInstance* shader_instance, const char* name, SkaVector4 value);

void ska_shader_instance_param_update_bool(SkaShaderInstance* shader_instance, const char* name, bool value);
void ska_shader_instance_param_update_int(SkaShaderInstance* shader_instance, const char* name, int32 value);
void ska_shader_instance_param_update_float(SkaShaderInstance* shader_instance, const char* name, f32 value);
void ska_shader_instance_param_update_float2(SkaShaderInstance* shader_instance, const char* name, SkaVector2 value);
void ska_shader_instance_param_update_float3(SkaShaderInstance* shader_instance, const char* name, SkaVector3 value);
void ska_shader_instance_param_update_float4(SkaShaderInstance* shader_instance, const char* name, SkaVector4 value);

bool ska_shader_instance_param_get_bool(SkaShaderInstance* shader_instance, const char* name);
int32 ska_shader_instance_param_get_int(SkaShaderInstance* shader_instance, const char* name);
f32 ska_shader_instance_param_get_float(SkaShaderInstance* shader_instance, const char* name);
SkaVector2 ska_shader_instance_param_get_float2(SkaShaderInstance* shader_instance, const char* name);
SkaVector3 ska_shader_instance_param_get_float3(SkaShaderInstance* shader_instance, const char* name);
SkaVector4 ska_shader_instance_param_get_float4(SkaShaderInstance* shader_instance, const char* name);

#endif // #if SKA_RENDERING
