#include "frame_buffer.h"

#include "shader/shader.h"
#include "shader/shader_instance.h"
#include "shader/shader_source.h"
#include "seika/logger.h"
#include "seika/assert.h"

GLuint frameBuffer = -1;
GLuint textureColorBuffer = -1;
GLuint rbo = -1;
bool hasBeenInitialized = false;

static SkaShaderInstance defaultScreenShader;
static SkaShaderInstance* currentScreenShader = NULL;
static GLuint screenVAO = -1;
static GLuint screenVBO = -1;

static int32 screenTextureWidth = 800;
static int32 screenTextureHeight = 600;
static int32 resolutionWidth = 800;
static int32 resolutionHeight = 600;
static SkaFrameBufferViewportData cachedViewportData = { .position = { .x = 0, .y = 0 }, .size = { .w = 800, .h = 600 } };
static bool maintainAspectRatio = false;

SkaFrameBufferViewportData ska_frame_buffer_generate_viewport_data(int32 windowWidth, int32 windowHeight) {
    int32 framebufferWidth = windowWidth;
    int32 framebufferHeight = windowHeight;

    const f32 game_aspect_ratio = (f32)resolutionWidth / (f32)resolutionHeight;
    const f32 window_aspect_ratio = (f32)windowWidth / (f32)windowHeight;

    // Adjust the framebuffer width or height to match the window aspect ratio
    if (maintainAspectRatio && game_aspect_ratio != window_aspect_ratio) {
        framebufferHeight = (int32)((f32)windowWidth / game_aspect_ratio);
        if (framebufferHeight > windowHeight) {
            framebufferHeight = windowHeight;
            framebufferWidth = (int32)((f32)windowHeight * game_aspect_ratio);
        }
    }

    // Calculate the viewport dimensions
    const int32 viewportX = (windowWidth - framebufferWidth) / 2;
    const int32 viewportY = (windowHeight - framebufferHeight) / 2;
    const int32 viewportWidth = framebufferWidth;
    const int32 viewportHeight = framebufferHeight;

    const SkaFrameBufferViewportData data = {
        .position = { .x = viewportX, .y = viewportY },
        .size = { .w = viewportWidth, .h = viewportHeight }
    };
    cachedViewportData = data;
    return data;
}

SkaFrameBufferViewportData* ska_frame_buffer_get_cached_viewport_data() {
    return &cachedViewportData;
}

bool recreate_frame_buffer_object() {
    // Create Framebuffer
    glGenFramebuffers(1, &frameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
    // Create color attachment
    glGenTextures(1, &textureColorBuffer);
    glBindTexture(GL_TEXTURE_2D, textureColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, screenTextureWidth, screenTextureHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorBuffer, 0);
    // Create renderbuffer object for depth and stencil attachment
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, screenTextureWidth, screenTextureHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
    // Check if framebuffer is complete
    const bool success = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
    if (!success) {
        ska_logger_error("Framebuffer is not complete!");
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return success;
}

bool ska_frame_buffer_initialize(int32 inWindowWidth, int32 inWindowHeight, int32 inResolutionWidth, int32 inResolutionHeight) {
    screenTextureWidth = inWindowWidth;
    screenTextureHeight = inWindowHeight;
    resolutionWidth = inResolutionWidth;
    resolutionHeight = inResolutionHeight;
    // VAO & VBO
    // Initialize render data
    glGenVertexArrays(1, &screenVAO);
    glGenBuffers(1, &screenVBO);
    glBindVertexArray(screenVAO);
    glBindBuffer(GL_ARRAY_BUFFER, screenVBO);
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
    SkaShader* screenShader = ska_shader_compile_new_shader(SKA_OPENGL_SHADER_SOURCE_VERTEX_SCREEN,
                                                            SKA_OPENGL_SHADER_SOURCE_FRAGMENT_SCREEN);
    defaultScreenShader = (SkaShaderInstance) {
        .shader = screenShader, .paramMap = ska_string_hash_map_create_default_capacity()
    };
    ska_frame_buffer_set_screen_shader(&defaultScreenShader);
    ska_frame_buffer_generate_viewport_data(inWindowWidth, inWindowHeight);

    hasBeenInitialized = true;
    return success;
}

void ska_frame_buffer_finalize() {
    ska_string_hash_map_destroy(defaultScreenShader.paramMap);
    defaultScreenShader.paramMap = NULL;
    hasBeenInitialized = false;
}

void ska_frame_buffer_bind() {
    SKA_ASSERT(hasBeenInitialized);
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
}

void ska_frame_buffer_unbind() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

uint32 ska_frame_buffer_get_color_buffer_texture() {
    return textureColorBuffer;
}

uint32 ska_frame_buffer_get_quad_vao() {
    return screenVAO;
}

void ska_frame_buffer_resize_texture(int32 newWidth, int32 newHeight) {
    screenTextureWidth = newWidth;
    screenTextureHeight = newHeight;
    recreate_frame_buffer_object();
}

void ska_frame_buffer_set_maintain_aspect_ratio(bool shouldMaintainAspectRatio) {
    maintainAspectRatio = shouldMaintainAspectRatio;
}

SkaShaderInstance* ska_frame_buffer_get_screen_shader() {
    return currentScreenShader;
}

void ska_frame_buffer_set_screen_shader(struct SkaShaderInstance *shaderInstance) {
    SKA_ASSERT_FMT(shaderInstance != NULL, "Trying to set screen shader to NULL!");
    currentScreenShader = shaderInstance;
    ska_shader_use(currentScreenShader->shader);
    ska_shader_set_int(currentScreenShader->shader, "TEXTURE", 0);
}

void ska_frame_buffer_reset_to_default_screen_shader() {
    currentScreenShader = &defaultScreenShader;
}
