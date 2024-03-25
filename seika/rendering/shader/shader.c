#include "shader.h"

#include "seika/logger.h"
#include "seika/memory.h"

static bool shader_check_compile_errors(uint32 shaderId, const char* type);

SkaShader* ska_shader_compile_new_shader(const char* vertexSource, const char* fragmentSource) {
    SkaShader* shader = SKA_MEM_ALLOCATE(SkaShader);
    GLuint vertex, fragment;
    // vertex
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vertexSource, NULL);
    glCompileShader(vertex);
    shader_check_compile_errors(vertex, SKA_SHADER_VERTEX_TYPE);
    // fragment
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fragmentSource, NULL);
    glCompileShader(fragment);
    if (!shader_check_compile_errors(vertex, SKA_SHADER_FRAGMENT_TYPE)) {
        return NULL;
    }
    // attack and link shaders
    shader->id = glCreateProgram();
    glAttachShader(shader->id, vertex);
    glAttachShader(shader->id, fragment);
    glLinkProgram(shader->id);
    if (!shader_check_compile_errors(shader->id, SKA_SHADER_PROGRAM_TYPE)) {
        return NULL;
    }
    glDeleteShader(vertex);
    glDeleteShader(fragment);
    return shader;
}

void ska_shader_destroy(SkaShader* shader) {
    SKA_MEM_FREE(shader);
}

bool shader_check_compile_errors(uint32 shaderId, const char* type) {
    GLint success;
    char infoLog[1024];
    if(type == SKA_SHADER_PROGRAM_TYPE) {
        glGetProgramiv(shaderId, GL_LINK_STATUS, &success);
        if(!success) {
            glGetProgramInfoLog(shaderId, 1024, NULL, infoLog);
            ska_logger_error("Shader type '%s' linking failed!\nInfoLog = %s", type, &infoLog);
        }
    } else {
        glGetShaderiv(shaderId, GL_COMPILE_STATUS, &success);
        if(!success) {
            glGetShaderInfoLog(shaderId, 1024, NULL, infoLog);
            ska_logger_error("Shader type '%s' compilation failed!InfoLog = \n%s", type, &infoLog);
        }
    }
    return success;
}

void ska_shader_use(SkaShader* shader) {
    glUseProgram(shader->id);
}

void ska_shader_set_bool(SkaShader* shader, const char* name, bool value) {
    glUniform1i(glGetUniformLocation(shader->id, name), (int32)value);
}

void ska_shader_set_int(SkaShader* shader, const char* name, int32 value) {
    glUniform1i(glGetUniformLocation(shader->id, name), value);
}

void ska_shader_set_float(SkaShader* shader, const char* name, f32 value) {
    glUniform1f(glGetUniformLocation(shader->id, name), value);
}

void ska_shader_set_vec2_float(SkaShader* shader, const char* name, f32 v1, f32 v2) {
    glUniform2f(glGetUniformLocation(shader->id, name), v1, v2);
}

void ska_shader_set_vec3_float(SkaShader* shader, const char* name, f32 v1, f32 v2, f32 v3) {
    glUniform3f(glGetUniformLocation(shader->id, name), v1, v2, v3);
}

void ska_shader_set_vec4_float(SkaShader* shader, const char* name, f32 v1, f32 v2, f32 v3, f32 v4) {
    glUniform4f(glGetUniformLocation(shader->id, name), v1, v2, v3, v4);
}

void ska_shader_set_mat4_float(SkaShader* shader, const char* name, mat4* value) {
    glUniformMatrix4fv(glGetUniformLocation(shader->id, name), 1, GL_FALSE, (f32*)value);
}
