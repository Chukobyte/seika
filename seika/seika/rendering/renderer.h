#pragma once

// OpenGL Renderer

#if SKA_RENDERING

#ifdef __cplusplus
extern "C" {
#endif

#include "texture.h"
#include "font.h"
#include "mesh.h"
#include "shader/shader_instance.h"
#include "seika/math/math.h"

#define SKA_RENDERER_MAX_Z_INDEX 100

typedef struct SkaRendererTransform2D {
    SkaMatrix4 model;
} SkaRendererTransform2D;

typedef struct SkaRenderer3DInitProps {
    f32 fov; // degrees
    f32 nearPlane;
    f32 farPlane;
} SkaRenderer3DInitProps;

typedef struct SkaRendererInitProps {
    int32 windowWidth;
    int32 windowHeight;
    int32 resolutionWidth;
    int32 resolutionHeight;
    bool maintainAspectRatio;
    bool init2D;
    SkaRenderer3DInitProps* initProps3D;
} SkaRendererInitProps;

void ska_renderer_init(SkaRendererInitProps props);
void ska_renderer_deinit();
void ska_renderer_clear(f32 red, f32 green, f32 blue, f32 alpha);
void ska_renderer_update_window_size(int32 windowWidth, int32 windowHeight);
void ska_renderer_set_sprite_shader_default_params(SkaShader* shader);

void ska_renderer_process_and_flush_batches(const SkaColor* backgroundColor);
void ska_renderer_process_and_flush_batches_just_framebuffer(const SkaColor* backgroundColor);

void ska_renderer_print_opengl_errors();

// Shader params
void ska_renderer_set_global_shader_param_time(f32 timeValue);
void ska_renderer_set_shader_instance_params(SkaShaderInstance* shaderInstance);

// 2D

void ska_renderer_init_2d();
void ska_renderer_deinit_2d();
void ska_renderer_queue_sprite_draw(SkaTexture* texture, SkaRect2 sourceRect, SkaSize2D destSize, SkaColor color, bool flipH, bool flipV, const SkaTransform2D* transform2D, int32 zIndex, SkaShaderInstance* shaderInstance);
void ska_renderer_queue_sprite_draw2(SkaTexture* texture, SkaRect2 sourceRect, SkaSize2D destSize, SkaColor color, bool flipH, bool flipV, SkaMatrix4* trsMatrix, int32 zIndex, SkaShaderInstance* shaderInstance);
void ska_renderer_queue_font_draw_call(SkaFont* font, const char* text, f32 x, f32 y, f32 scale, SkaColor color, int32 zIndex);
void ska_renderer_flush_2d_batches();

// 3D

typedef struct SkaDirectionalLightQueueProps {
    SkaVector3 direction;
    SkaVector3 ambient;
    SkaVector3 diffuse;
    SkaVector3 specular;
    int32 zIndex;
    SkaShaderInstance* shaderInstance;
} SkaDirectionalLightQueueProps;

typedef struct SkaPointLightQueueProps {
    SkaVector3 position;
    SkaVector3 scale;
    SkaVector3 ambient;
    SkaVector3 diffuse;
    SkaVector3 specular;
    float linear;
    float quadratic;
    const float constant;
    int32 zIndex;
    SkaShaderInstance* shaderInstance;
} SkaPointLightQueueProps;

typedef struct SkaSpotLightQueueProps {
    SkaVector3 position;
    SkaVector3 direction;
    SkaVector3 ambient;
    SkaVector3 diffuse;
    SkaVector3 specular;
    float linear;
    float quadratic;
    const float constant;
    bool isAttachedToCamera;
    float cutoff;
    float outerCutoff;
    int32 zIndex;
    SkaShaderInstance* shaderInstance;
} SkaSpotLightQueueProps;

typedef struct SkaRendererTransform3D {
    SkaMatrix4 model;
    SkaMatrix4 view;
    SkaMatrix4 projection;
} SkaRendererTransform3D;

typedef struct SkaCamera3D {
    SkaVector3 position;
    SkaVector3 rotation;
    SkaVector3 up;
    SkaVector3 front;
    f32 fov;
    f32 nearPlane;
    f32 farPlane;
} SkaCamera3D;

#define SKA_DEFAULT_CAMERA_3D (SkaCamera3D) { .position = SKA_VECTOR3_ZERO, .rotation = SKA_VECTOR3_ZERO, .up = SKA_VECTOR3_UP, .front = SKA_VECTOR3_ZERO, .fov = 45.0f, .nearPlane = 0.1f, .farPlane = 1000.0f }

// 3D Rendering functions
void ska_renderer_init_3d(SkaRenderer3DInitProps props);
void ska_renderer_set_view_matrix(const SkaMatrix4* view);
void ska_renderer_set_projection_matrix(const SkaMatrix4* projection);
void ska_renderer_queue_mesh_draw(SkaMesh* mesh, const SkaMatrix4* model, SkaShaderInstance* shaderInstance);
void ska_renderer_queue_directional_light_draw(SkaDirectionalLightQueueProps props);
void ska_renderer_queue_point_light_draw(SkaPointLightQueueProps props);
void ska_renderer_queue_spot_light_draw(SkaSpotLightQueueProps props);
void ska_renderer_draw_model(SkaModel* model, const SkaMatrix4* modelMatrix, SkaShaderInstance* shaderInstance);
void ska_renderer_flush_meshes();
void ska_renderer_flush_lights();

void ska_renderer_set_camera_position(SkaVector3 position);
SkaCamera3D* ska_renderer_get_active_camera();
void ska_renderer_get_camera_view_matrix(const SkaCamera3D* camera, SkaMatrix4* matrix4);
void ska_renderer_get_camera_projection_matrix(const SkaCamera3D* camera, SkaMatrix4* matrix4);

#ifdef __cplusplus
}
#endif

#endif // #if SKA_RENDERING
