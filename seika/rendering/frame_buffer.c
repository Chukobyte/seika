#if SKA_RENDERING

#include "frame_buffer.h"

#include "shader/shader.h"
#include "shader/shader_instance.h"
#include "shader/shader_source.h"
#include "../logger.h"
#include "seika/assert.h"

GLuint frame_buffer = -1;
GLuint texture_color_buffer = -1;
GLuint rbo = -1;
bool has_been_initialized = false;

static SkaShaderInstance default_screen_shader;
static SkaShaderInstance* current_screen_shader = NULL;
static GLuint screen_vao = -1;
static GLuint screen_vbo = -1;

static int32 screen_texture_width = 800;
static int32 screen_texture_height = 600;
static int32 resolution_width = 800;
static int32 resolution_height = 600;
static SkaFrameBufferViewportData cached_viewport_data = { .position = { .x = 0, .y = 0 }, .size = { .w = 800, .h = 600 } };
static bool maintain_aspect_ratio = false;

SkaFrameBufferViewportData ska_frame_buffer_generate_viewport_data(int32 window_width, int32 window_height) {
    int32 framebuffer_width = window_width;
    int32 framebuffer_height = window_height;

    const f32 game_aspect_ratio = (f32)resolution_width / (f32)resolution_height;
    const f32 window_aspect_ratio = (f32)window_width / (f32)window_height;

    // Adjust the framebuffer width or height to match the window aspect ratio
    if (maintain_aspect_ratio && game_aspect_ratio != window_aspect_ratio) {
        framebuffer_height = (int32)((f32)window_width / game_aspect_ratio);
        if (framebuffer_height > window_height) {
            framebuffer_height = window_height;
            framebuffer_width = (int32)((f32)window_height * game_aspect_ratio);
        }
    }

    // Calculate the viewport dimensions
    const int32 viewport_x = (window_width - framebuffer_width) / 2;
    const int32 viewport_y = (window_height - framebuffer_height) / 2;
    const int32 viewport_width = framebuffer_width;
    const int32 viewport_height = framebuffer_height;

    const SkaFrameBufferViewportData data = {
        .position = { .x = viewport_x, .y = viewport_y },
        .size = { .w = viewport_width, .h = viewport_height }
    };
    cached_viewport_data = data;
    return data;
}

SkaFrameBufferViewportData* ska_frame_buffer_get_cached_viewport_data() {
    return &cached_viewport_data;
}

SkaVector2 ska_frame_buffer_window_to_resolution(SkaVector2i window_position) {
    const SkaFrameBufferViewportData* vp = &cached_viewport_data;
    if (vp->size.w <= 0 || vp->size.h <= 0) {
        return (SkaVector2){ (f32)window_position.x, (f32)window_position.y };
    }
    return (SkaVector2){
        (f32)(window_position.x - vp->position.x) * (f32)resolution_width / (f32)vp->size.w,
        (f32)(window_position.y - vp->position.y) * (f32)resolution_height / (f32)vp->size.h,
    };
}

bool recreate_frame_buffer_object() {
    // Clean up old objects if they exist
    if (frame_buffer != (GLuint)-1) {
        glDeleteFramebuffers(1, &frame_buffer);
    }
    if (texture_color_buffer != (GLuint)-1) {
        glDeleteTextures(1, &texture_color_buffer);
    }
    if (rbo != (GLuint)-1) {
        glDeleteRenderbuffers(1, &rbo);
    }

    // Create Framebuffer
    glGenFramebuffers(1, &frame_buffer);
    glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
    // Create color attachment
    glGenTextures(1, &texture_color_buffer);
    glBindTexture(GL_TEXTURE_2D, texture_color_buffer);
    // internalformat must match format exactly for unsized formats under GLES/WebGL (desktop GL
    // tolerates the RGB/RGBA mismatch that used to be here; GLES doesn't -- glTexImage2D silently
    // fails to allocate storage, leaving the texture 0x0 and the framebuffer attachment incomplete).
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, screen_texture_width, screen_texture_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_color_buffer, 0);
    // Create renderbuffer object for depth and stencil attachment
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, screen_texture_width, screen_texture_height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
    // Check if framebuffer is complete
    const bool success = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
    if (!success) {
        ska_logger_error("Framebuffer is not complete!");
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return success;
}

bool ska_frame_buffer_initialize(int32 in_window_width, int32 in_window_height, int32 in_resolution_width, int32 in_resolution_height) {
    screen_texture_width = in_window_width;
    screen_texture_height = in_window_height;
    resolution_width = in_resolution_width;
    resolution_height = in_resolution_height;
    // VAO & VBO
    // Initialize render data
    glGenVertexArrays(1, &screen_vao);
    glGenBuffers(1, &screen_vbo);
    glBindVertexArray(screen_vao);
    glBindBuffer(GL_ARRAY_BUFFER, screen_vbo);
    // Set buffer data
    GLfloat vertices[] = {
        // pos      // tex coords
        -1.0f, 1.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f,
            1.0f, -1.0f, 1.0f, 0.0f,

            -1.0f, 1.0f, 0.0f, 1.0f,
            1.0f, -1.0f, 1.0f, 0.0f,
            1.0f, 1.0f, 1.0f, 1.0f
        };
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    // position attribute
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*) NULL);
    // texture coords attribute
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)(2 * sizeof(GLfloat)));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    bool success = recreate_frame_buffer_object();

    // compile shaders
    SkaShader* screen_shader = ska_shader_compile_new_shader(ska_opengl_shader_source_vertex_screen,
                                                            ska_opengl_shader_source_fragment_screen);
    default_screen_shader = (SkaShaderInstance){
        .shader = screen_shader,
        .param_map = ska_string_hash_map_create_default_capacity()};
    ska_frame_buffer_set_screen_shader(&default_screen_shader);
    ska_frame_buffer_generate_viewport_data(in_window_width, in_window_height);

    has_been_initialized = true;
    return success;
}

void ska_frame_buffer_finalize() {
    ska_string_hash_map_deinit(default_screen_shader.param_map);
    default_screen_shader.param_map = NULL;
    has_been_initialized = false;
}

void ska_frame_buffer_bind() {
    SKA_ASSERT(has_been_initialized);
    glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
}

void ska_frame_buffer_unbind() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

uint32 ska_frame_buffer_get_color_buffer_texture() {
    return texture_color_buffer;
}

uint32 ska_frame_buffer_get_quad_vao() {
    return screen_vao;
}

void ska_frame_buffer_resize_texture(int32 new_width, int32 new_height) {
    screen_texture_width = new_width;
    screen_texture_height = new_height;
    recreate_frame_buffer_object();
}

void ska_frame_buffer_set_maintain_aspect_ratio(bool should_maintain_aspect_ratio) {
    maintain_aspect_ratio = should_maintain_aspect_ratio;
}

SkaShaderInstance* ska_frame_buffer_get_screen_shader() {
    return current_screen_shader;
}

void ska_frame_buffer_set_screen_shader(struct SkaShaderInstance *shader_instance) {
    SKA_ASSERT_FMT(shader_instance != NULL, "Trying to set screen shader to NULL!");
    current_screen_shader = shader_instance;
    ska_shader_use(current_screen_shader->shader);
    ska_shader_set_int(current_screen_shader->shader, "TEXTURE", 0);
}

void ska_frame_buffer_reset_to_default_screen_shader() {
    current_screen_shader = &default_screen_shader;
}

#endif // #if SKA_RENDERING