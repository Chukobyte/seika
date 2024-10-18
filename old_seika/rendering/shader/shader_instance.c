#include "shader_instance.h"

#include <string.h>
#include <stdbool.h>

#include "seika/memory.h"
#include "seika/logger.h"
#include "seika/string.h"
#include "seika/assert.h"

SkaShaderInstance* ska_shader_instance_create(const char* vertexSource, const char* fragmentSource) {
    SkaShader* shader = ska_shader_compile_new_shader(vertexSource, fragmentSource);
    // Early out if shader fails to compile
    if (shader == NULL) {
        return NULL;
    }
    SkaShaderInstance* instance = SKA_MEM_ALLOCATE(SkaShaderInstance);
    instance->shader = shader;
    instance->paramMap = ska_string_hash_map_create_default_capacity();
    instance->paramsDirty = true;
    return instance;
}

SkaShaderInstance* ska_shader_instance_create_from_shader(SkaShader* shader) {
    SKA_ASSERT(shader != NULL);
    SkaShaderInstance* instance = SKA_MEM_ALLOCATE(SkaShaderInstance);
    instance->shader = shader;
    instance->paramMap = ska_string_hash_map_create_default_capacity();
    instance->paramsDirty = true;
    return instance;
}

void ska_shader_instance_destroy(SkaShaderInstance* shaderInstance) {
    SKA_STRING_HASH_MAP_FOR_EACH(shaderInstance->paramMap, iter) {
        SkaStringHashMapNode* node = iter.pair;
        SkaShaderParam* param = (SkaShaderParam*)node->value;
        SKA_MEM_FREE(param->name);
    }
    ska_string_hash_map_destroy(shaderInstance->paramMap);
    ska_shader_destroy(shaderInstance->shader);
    SKA_MEM_FREE(shaderInstance);
}

// Creation functions
void ska_shader_instance_param_create_from_copy(SkaShaderInstance* shaderInstance, SkaShaderParam* param) {
    param->name = ska_strdup(param->name);
    switch (param->type) {
    case SkaShaderParamType_BOOL: {
        ska_shader_instance_param_create_bool(shaderInstance, param->name, param->value.boolValue);
        return;
    }
    case SkaShaderParamType_INT: {
        ska_shader_instance_param_create_int(shaderInstance, param->name, param->value.intValue);
        return;
    }
    case SkaShaderParamType_FLOAT: {
        ska_shader_instance_param_create_float(shaderInstance, param->name, param->value.floatValue);
        return;
    }
    case SkaShaderParamType_FLOAT2: {
        ska_shader_instance_param_create_float2(shaderInstance, param->name, param->value.float2Value);
        return;
    }
    case SkaShaderParamType_FLOAT3: {
        ska_shader_instance_param_create_float3(shaderInstance, param->name, param->value.float3Value);
        return;
    }
    case SkaShaderParamType_FLOAT4: {
        ska_shader_instance_param_create_float4(shaderInstance, param->name, param->value.float4Value);
        return;
    }
    }
    SKA_ASSERT_FMT(false, "Failed to copy shader param with name '%s'", param->name);
}

SkaShaderParam* ska_shader_instance_param_create_bool(SkaShaderInstance* shaderInstance, const char* name, bool value) {
    SkaShaderParam params = { .name = ska_strdup(name), .type = SkaShaderParamType_BOOL };
    params.value.boolValue = value;
    ska_string_hash_map_add(shaderInstance->paramMap, name, &params, sizeof(SkaShaderParam));
    return (SkaShaderParam*)ska_string_hash_map_get(shaderInstance->paramMap, name);
}

SkaShaderParam* ska_shader_instance_param_create_int(SkaShaderInstance* shaderInstance, const char* name, int32 value) {
    SkaShaderParam params = { .name = ska_strdup(name), .type = SkaShaderParamType_INT };
    params.value.intValue = value;
    ska_string_hash_map_add(shaderInstance->paramMap, name, &params, sizeof(SkaShaderParam));
    return (SkaShaderParam*)ska_string_hash_map_get(shaderInstance->paramMap, name);
}

SkaShaderParam* ska_shader_instance_param_create_float(SkaShaderInstance* shaderInstance, const char* name, f32 value) {
    SkaShaderParam params = { .name = ska_strdup(name), .type = SkaShaderParamType_FLOAT };
    params.value.floatValue = value;
    ska_string_hash_map_add(shaderInstance->paramMap, name, &params, sizeof(SkaShaderParam));
    return (SkaShaderParam*)ska_string_hash_map_get(shaderInstance->paramMap, name);
}

SkaShaderParam* ska_shader_instance_param_create_float2(SkaShaderInstance* shaderInstance, const char* name, SkaVector2 value) {
    SkaShaderParam params = { .name = ska_strdup(name), .type = SkaShaderParamType_FLOAT2 };
    params.value.float2Value = value;
    ska_string_hash_map_add(shaderInstance->paramMap, name, &params, sizeof(SkaShaderParam));
    return (SkaShaderParam*)ska_string_hash_map_get(shaderInstance->paramMap, name);
}

SkaShaderParam* ska_shader_instance_param_create_float3(SkaShaderInstance* shaderInstance, const char* name, SkaVector3 value) {
    SkaShaderParam params = { .name = ska_strdup(name), .type = SkaShaderParamType_FLOAT3 };
    params.value.float3Value = value;
    ska_string_hash_map_add(shaderInstance->paramMap, name, &params, sizeof(SkaShaderParam));
    return (SkaShaderParam*)ska_string_hash_map_get(shaderInstance->paramMap, name);
}

SkaShaderParam* ska_shader_instance_param_create_float4(SkaShaderInstance* shaderInstance, const char* name, SkaVector4 value) {
    SkaShaderParam params = { .name = ska_strdup(name), .type = SkaShaderParamType_FLOAT4 };
    params.value.float4Value = value;
    ska_string_hash_map_add(shaderInstance->paramMap, name, &params, sizeof(SkaShaderParam));
    return (SkaShaderParam*)ska_string_hash_map_get(shaderInstance->paramMap, name);
}

// Update functions
void ska_shader_instance_param_update_bool(SkaShaderInstance* shaderInstance, const char* name, bool value) {
    SkaShaderParam* param = (SkaShaderParam*)ska_string_hash_map_get(shaderInstance->paramMap, name);
    SKA_ASSERT(param->type == SkaShaderParamType_BOOL);
    param->value.boolValue = value;
    shaderInstance->paramsDirty = true;
}

void ska_shader_instance_param_update_int(SkaShaderInstance* shaderInstance, const char* name, int32 value) {
    SkaShaderParam* param = (SkaShaderParam*)ska_string_hash_map_get(shaderInstance->paramMap, name);
    SKA_ASSERT(param->type == SkaShaderParamType_INT);
    param->value.intValue = value;
    shaderInstance->paramsDirty = true;
}

void ska_shader_instance_param_update_float(SkaShaderInstance* shaderInstance, const char* name, f32 value) {
    SkaShaderParam* param = (SkaShaderParam*)ska_string_hash_map_get(shaderInstance->paramMap, name);
    SKA_ASSERT_FMT(param != NULL, "Shader param for '%s' is null!", name);
    SKA_ASSERT(param->type == SkaShaderParamType_FLOAT);
    param->value.floatValue = value;
    shaderInstance->paramsDirty = true;
}

void ska_shader_instance_param_update_float2(SkaShaderInstance* shaderInstance, const char* name, SkaVector2 value) {
    SkaShaderParam* param = (SkaShaderParam*)ska_string_hash_map_get(shaderInstance->paramMap, name);
    SKA_ASSERT(param->type == SkaShaderParamType_FLOAT2);
    param->value.float2Value = value;
    shaderInstance->paramsDirty = true;
}

void ska_shader_instance_param_update_float3(SkaShaderInstance* shaderInstance, const char* name, SkaVector3 value) {
    SkaShaderParam* param = (SkaShaderParam*)ska_string_hash_map_get(shaderInstance->paramMap, name);
    SKA_ASSERT(param->type == SkaShaderParamType_FLOAT3);
    param->value.float3Value = value;
    shaderInstance->paramsDirty = true;
}

void ska_shader_instance_param_update_float4(SkaShaderInstance* shaderInstance, const char* name, SkaVector4 value) {
    SkaShaderParam* param = (SkaShaderParam*)ska_string_hash_map_get(shaderInstance->paramMap, name);
    SKA_ASSERT(param->type == SkaShaderParamType_FLOAT4);
    param->value.float4Value = value;
    shaderInstance->paramsDirty = true;
}

// Get functions
bool ska_shader_instance_param_get_bool(SkaShaderInstance* shaderInstance, const char* name) {
    SkaShaderParam* param = (SkaShaderParam*)ska_string_hash_map_get(shaderInstance->paramMap, name);
    SKA_ASSERT(param->type == SkaShaderParamType_BOOL);
    return param->value.boolValue;
}

int32 ska_shader_instance_param_get_int(SkaShaderInstance* shaderInstance, const char* name) {
    SkaShaderParam* param = (SkaShaderParam*)ska_string_hash_map_get(shaderInstance->paramMap, name);
    SKA_ASSERT(param->type == SkaShaderParamType_INT);
    return param->value.intValue;
}

f32 ska_shader_instance_param_get_float(SkaShaderInstance* shaderInstance, const char* name) {
    SkaShaderParam* param = (SkaShaderParam*)ska_string_hash_map_get(shaderInstance->paramMap, name);
    SKA_ASSERT(param->type == SkaShaderParamType_FLOAT);
    return param->value.floatValue;
}

SkaVector2 ska_shader_instance_param_get_float2(SkaShaderInstance* shaderInstance, const char* name) {
    SkaShaderParam* param = (SkaShaderParam*)ska_string_hash_map_get(shaderInstance->paramMap, name);
    SKA_ASSERT(param->type == SkaShaderParamType_FLOAT2);
    return param->value.float2Value;
}

SkaVector3 ska_shader_instance_param_get_float3(SkaShaderInstance* shaderInstance, const char* name) {
    SkaShaderParam* param = (SkaShaderParam*)ska_string_hash_map_get(shaderInstance->paramMap, name);
    SKA_ASSERT(param->type == SkaShaderParamType_FLOAT3);
    return param->value.float3Value;
}

SkaVector4 ska_shader_instance_param_get_float4(SkaShaderInstance* shaderInstance, const char* name) {
    SkaShaderParam* param = (SkaShaderParam*)ska_string_hash_map_get(shaderInstance->paramMap, name);
    SKA_ASSERT(param->type == SkaShaderParamType_FLOAT4);
    return param->value.float4Value;
}
