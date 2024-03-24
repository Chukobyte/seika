#pragma once

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
        bool boolValue;
        int32 intValue;
        f32 floatValue;
        SkaVector2 float2Value;
        SkaVector3 float3Value;
        SkaVector4 float4Value;
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
    SkaStringHashMap* paramMap;
    bool paramsDirty;
} SkaShaderInstance;

SkaShaderInstance* ska_shader_instance_create(const char* vertexSource, const char* fragmentSource);
SkaShaderInstance* ska_shader_instance_create_from_shader(SkaShader* shader);
void ska_shader_instance_destroy(SkaShaderInstance* shaderInstance);

void ska_shader_instance_param_create_from_copy(SkaShaderInstance* shaderInstance, SkaShaderParam* param);
SkaShaderParam* ska_shader_instance_param_create_bool(SkaShaderInstance* shaderInstance, const char* name, bool value);
SkaShaderParam* ska_shader_instance_param_create_int(SkaShaderInstance* shaderInstance, const char* name, int32 value);
SkaShaderParam* ska_shader_instance_param_create_float(SkaShaderInstance* shaderInstance, const char* name, f32 value);
SkaShaderParam* ska_shader_instance_param_create_float2(SkaShaderInstance* shaderInstance, const char* name, SkaVector2 value);
SkaShaderParam* ska_shader_instance_param_create_float3(SkaShaderInstance* shaderInstance, const char* name, SkaVector3 value);
SkaShaderParam* ska_shader_instance_param_create_float4(SkaShaderInstance* shaderInstance, const char* name, SkaVector4 value);

void ska_shader_instance_param_update_bool(SkaShaderInstance* shaderInstance, const char* name, bool value);
void ska_shader_instance_param_update_int(SkaShaderInstance* shaderInstance, const char* name, int32 value);
void ska_shader_instance_param_update_float(SkaShaderInstance* shaderInstance, const char* name, f32 value);
void ska_shader_instance_param_update_float2(SkaShaderInstance* shaderInstance, const char* name, SkaVector2 value);
void ska_shader_instance_param_update_float3(SkaShaderInstance* shaderInstance, const char* name, SkaVector3 value);
void ska_shader_instance_param_update_float4(SkaShaderInstance* shaderInstance, const char* name, SkaVector4 value);

bool ska_shader_instance_param_get_bool(SkaShaderInstance* shaderInstance, const char* name);
int32 ska_shader_instance_param_get_int(SkaShaderInstance* shaderInstance, const char* name);
f32 ska_shader_instance_param_get_float(SkaShaderInstance* shaderInstance, const char* name);
SkaVector2 ska_shader_instance_param_get_float2(SkaShaderInstance* shaderInstance, const char* name);
SkaVector3 ska_shader_instance_param_get_float3(SkaShaderInstance* shaderInstance, const char* name);
SkaVector4 ska_shader_instance_param_get_float4(SkaShaderInstance* shaderInstance, const char* name);
