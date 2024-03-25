#pragma once

#include <stdbool.h>

#include <glad/glad.h>

#include "seika/math/math.h"

static const char* SKA_SHADER_VERTEX_TYPE = "VERTEX";
static const char* SKA_SHADER_FRAGMENT_TYPE = "FRAGMENT";
static const char* SKA_SHADER_PROGRAM_TYPE = "PROGRAM";

typedef struct SkaShader {
    GLuint id;
} SkaShader;

SkaShader* ska_shader_compile_new_shader(const char* vertexSource, const char* fragmentSource);
void ska_shader_destroy(SkaShader* shader);
void ska_shader_use(SkaShader* shader);
void ska_shader_set_bool(SkaShader* shader, const char* name, bool value);
void ska_shader_set_int(SkaShader* shader, const char* name, int32 value);
void ska_shader_set_float(SkaShader* shader, const char* name, f32 value);
void ska_shader_set_vec2_float(SkaShader* shader, const char* name, f32 v1, f32 v2);
void ska_shader_set_vec3_float(SkaShader* shader, const char* name, f32 v1, f32 v2, f32 v3);
void ska_shader_set_vec4_float(SkaShader* shader, const char* name, f32 v1, f32 v2, f32 v3, f32 v4);
void ska_shader_set_mat4_float(SkaShader* shader, const char* name, mat4* value);
