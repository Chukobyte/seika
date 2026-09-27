#if SKA_RENDERING

#include "shader_instance.h"

#include <string.h>
#include <stdbool.h>

#include "seika/memory.h"
#include "seika/string.h"
#include "seika/assert.h"

SkaShaderInstance* ska_shader_instance_create(const char* vertex_source, const char* fragment_source) {
    SkaShader* shader = ska_shader_compile_new_shader(vertex_source, fragment_source);
    // Early out if shader fails to compile
    if (shader == NULL) {
        return NULL;
    }
    SkaShaderInstance* instance = SKA_ALLOC(SkaShaderInstance);
    instance->shader = shader;
    instance->param_map = ska_string_hash_map_create_default_capacity();
    instance->params_dirty = true;
    return instance;
}

SkaShaderInstance* ska_shader_instance_create_from_shader(SkaShader* shader) {
    SKA_ASSERT(shader != NULL);
    SkaShaderInstance* instance = SKA_ALLOC(SkaShaderInstance);
    instance->shader = shader;
    instance->param_map = ska_string_hash_map_create_default_capacity();
    instance->params_dirty = true;
    return instance;
}

void ska_shader_instance_destroy(SkaShaderInstance* shader_instance) {
    SKA_STRING_HASH_MAP_FOR_EACH(shader_instance->param_map, iter) {
        SkaShaderParam* param = (SkaShaderParam*)iter.value;
        SKA_FREE(param->name);
    }
    ska_string_hash_map_deinit(shader_instance->param_map);
    ska_shader_destroy(shader_instance->shader);
    SKA_FREE(shader_instance);
}

// Creation functions
void ska_shader_instance_param_create_from_copy(SkaShaderInstance* shader_instance, SkaShaderParam* param) {
    param->name = ska_strdup(param->name);
    switch (param->type) {
    case SkaShaderParamType_BOOL: {
        ska_shader_instance_param_create_bool(shader_instance, param->name, param->value.bool_value);
        return;
    }
    case SkaShaderParamType_INT: {
        ska_shader_instance_param_create_int(shader_instance, param->name, param->value.int_value);
        return;
    }
    case SkaShaderParamType_FLOAT: {
        ska_shader_instance_param_create_float(shader_instance, param->name, param->value.float_value);
        return;
    }
    case SkaShaderParamType_FLOAT2: {
        ska_shader_instance_param_create_float2(shader_instance, param->name, param->value.float2_value);
        return;
    }
    case SkaShaderParamType_FLOAT3: {
        ska_shader_instance_param_create_float3(shader_instance, param->name, param->value.float3_value);
        return;
    }
    case SkaShaderParamType_FLOAT4: {
        ska_shader_instance_param_create_float4(shader_instance, param->name, param->value.float4_value);
        return;
    }
    }
    SKA_ASSERT_FMT(false, "Failed to copy shader param with name '%s'", param->name);
}

SkaShaderParam* ska_shader_instance_param_create_bool(SkaShaderInstance* shader_instance, const char* name, bool value) {
    SkaShaderParam params = { .name = ska_strdup(name), .type = SkaShaderParamType_BOOL };
    params.value.bool_value = value;
    ska_string_hash_map_add(shader_instance->param_map, name, &params, sizeof(SkaShaderParam));
    return (SkaShaderParam*)ska_string_hash_map_get(shader_instance->param_map, name);
}

SkaShaderParam* ska_shader_instance_param_create_int(SkaShaderInstance* shader_instance, const char* name, int32 value) {
    SkaShaderParam params = { .name = ska_strdup(name), .type = SkaShaderParamType_INT };
    params.value.int_value = value;
    ska_string_hash_map_add(shader_instance->param_map, name, &params, sizeof(SkaShaderParam));
    return (SkaShaderParam*)ska_string_hash_map_get(shader_instance->param_map, name);
}

SkaShaderParam* ska_shader_instance_param_create_float(SkaShaderInstance* shader_instance, const char* name, f32 value) {
    SkaShaderParam params = { .name = ska_strdup(name), .type = SkaShaderParamType_FLOAT };
    params.value.float_value = value;
    ska_string_hash_map_add(shader_instance->param_map, name, &params, sizeof(SkaShaderParam));
    return (SkaShaderParam*)ska_string_hash_map_get(shader_instance->param_map, name);
}

SkaShaderParam* ska_shader_instance_param_create_float2(SkaShaderInstance* shader_instance, const char* name, SkaVector2 value) {
    SkaShaderParam params = { .name = ska_strdup(name), .type = SkaShaderParamType_FLOAT2 };
    params.value.float2_value = value;
    ska_string_hash_map_add(shader_instance->param_map, name, &params, sizeof(SkaShaderParam));
    return (SkaShaderParam*)ska_string_hash_map_get(shader_instance->param_map, name);
}

SkaShaderParam* ska_shader_instance_param_create_float3(SkaShaderInstance* shader_instance, const char* name, SkaVector3 value) {
    SkaShaderParam params = { .name = ska_strdup(name), .type = SkaShaderParamType_FLOAT3 };
    params.value.float3_value = value;
    ska_string_hash_map_add(shader_instance->param_map, name, &params, sizeof(SkaShaderParam));
    return (SkaShaderParam*)ska_string_hash_map_get(shader_instance->param_map, name);
}

SkaShaderParam* ska_shader_instance_param_create_float4(SkaShaderInstance* shader_instance, const char* name, SkaVector4 value) {
    SkaShaderParam params = { .name = ska_strdup(name), .type = SkaShaderParamType_FLOAT4 };
    params.value.float4_value = value;
    ska_string_hash_map_add(shader_instance->param_map, name, &params, sizeof(SkaShaderParam));
    return (SkaShaderParam*)ska_string_hash_map_get(shader_instance->param_map, name);
}

// Update functions
void ska_shader_instance_param_update_bool(SkaShaderInstance* shader_instance, const char* name, bool value) {
    SkaShaderParam* param = (SkaShaderParam*)ska_string_hash_map_get(shader_instance->param_map, name);
    SKA_ASSERT(param->type == SkaShaderParamType_BOOL);
    param->value.bool_value = value;
    shader_instance->params_dirty = true;
}

void ska_shader_instance_param_update_int(SkaShaderInstance* shader_instance, const char* name, int32 value) {
    SkaShaderParam* param = (SkaShaderParam*)ska_string_hash_map_get(shader_instance->param_map, name);
    SKA_ASSERT(param->type == SkaShaderParamType_INT);
    param->value.int_value = value;
    shader_instance->params_dirty = true;
}

void ska_shader_instance_param_update_float(SkaShaderInstance* shader_instance, const char* name, f32 value) {
    SkaShaderParam* param = (SkaShaderParam*)ska_string_hash_map_get(shader_instance->param_map, name);
    SKA_ASSERT_FMT(param != NULL, "Shader param for '%s' is null!", name);
    SKA_ASSERT(param->type == SkaShaderParamType_FLOAT);
    param->value.float_value = value;
    shader_instance->params_dirty = true;
}

void ska_shader_instance_param_update_float2(SkaShaderInstance* shader_instance, const char* name, SkaVector2 value) {
    SkaShaderParam* param = (SkaShaderParam*)ska_string_hash_map_get(shader_instance->param_map, name);
    SKA_ASSERT(param->type == SkaShaderParamType_FLOAT2);
    param->value.float2_value = value;
    shader_instance->params_dirty = true;
}

void ska_shader_instance_param_update_float3(SkaShaderInstance* shader_instance, const char* name, SkaVector3 value) {
    SkaShaderParam* param = (SkaShaderParam*)ska_string_hash_map_get(shader_instance->param_map, name);
    SKA_ASSERT(param->type == SkaShaderParamType_FLOAT3);
    param->value.float3_value = value;
    shader_instance->params_dirty = true;
}

void ska_shader_instance_param_update_float4(SkaShaderInstance* shader_instance, const char* name, SkaVector4 value) {
    SkaShaderParam* param = (SkaShaderParam*)ska_string_hash_map_get(shader_instance->param_map, name);
    SKA_ASSERT(param->type == SkaShaderParamType_FLOAT4);
    param->value.float4_value = value;
    shader_instance->params_dirty = true;
}

// Get functions
bool ska_shader_instance_param_get_bool(SkaShaderInstance* shader_instance, const char* name) {
    SkaShaderParam* param = (SkaShaderParam*)ska_string_hash_map_get(shader_instance->param_map, name);
    SKA_ASSERT(param->type == SkaShaderParamType_BOOL);
    return param->value.bool_value;
}

int32 ska_shader_instance_param_get_int(SkaShaderInstance* shader_instance, const char* name) {
    SkaShaderParam* param = (SkaShaderParam*)ska_string_hash_map_get(shader_instance->param_map, name);
    SKA_ASSERT(param->type == SkaShaderParamType_INT);
    return param->value.int_value;
}

f32 ska_shader_instance_param_get_float(SkaShaderInstance* shader_instance, const char* name) {
    SkaShaderParam* param = (SkaShaderParam*)ska_string_hash_map_get(shader_instance->param_map, name);
    SKA_ASSERT(param->type == SkaShaderParamType_FLOAT);
    return param->value.float_value;
}

SkaVector2 ska_shader_instance_param_get_float2(SkaShaderInstance* shader_instance, const char* name) {
    SkaShaderParam* param = (SkaShaderParam*)ska_string_hash_map_get(shader_instance->param_map, name);
    SKA_ASSERT(param->type == SkaShaderParamType_FLOAT2);
    return param->value.float2_value;
}

SkaVector3 ska_shader_instance_param_get_float3(SkaShaderInstance* shader_instance, const char* name) {
    SkaShaderParam* param = (SkaShaderParam*)ska_string_hash_map_get(shader_instance->param_map, name);
    SKA_ASSERT(param->type == SkaShaderParamType_FLOAT3);
    return param->value.float3_value;
}

SkaVector4 ska_shader_instance_param_get_float4(SkaShaderInstance* shader_instance, const char* name) {
    SkaShaderParam* param = (SkaShaderParam*)ska_string_hash_map_get(shader_instance->param_map, name);
    SKA_ASSERT(param->type == SkaShaderParamType_FLOAT4);
    return param->value.float4_value;
}

#endif // #if SKA_RENDERING
