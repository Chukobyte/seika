#pragma once

#if SKA_RENDERING

#include <stdbool.h>

#include "seika/rendering/ska_opengl.h"
#include "seika/math/math.h"

static const char* ska_shader_vertex_type = "VERTEX";
static const char* ska_shader_fragment_type = "FRAGMENT";
static const char* ska_shader_program_type = "PROGRAM";

typedef struct SkaShader {
    GLuint id;
} SkaShader;

SkaShader* ska_shader_compile_new_shader(const char* vertex_source, const char* fragment_source);
void ska_shader_destroy(SkaShader* shader);
void ska_shader_use(SkaShader* shader);
void ska_shader_set_bool(SkaShader* shader, const char* name, bool value);
void ska_shader_set_int(SkaShader* shader, const char* name, int32 value);
void ska_shader_set_float(SkaShader* shader, const char* name, f32 value);
void ska_shader_set_vec2_float(SkaShader* shader, const char* name, f32 v1, f32 v2);
void ska_shader_set_vec3_float(SkaShader* shader, const char* name, f32 v1, f32 v2, f32 v3);
void ska_shader_set_vec4_float(SkaShader* shader, const char* name, f32 v1, f32 v2, f32 v3, f32 v4);
void ska_shader_set_mat4_float(SkaShader* shader, const char* name, SkaMatrix4* value);

#endif // #if SKA_RENDERING
