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
    f32 near_plane;
    f32 far_plane;
} SkaRenderer3DInitProps;

typedef struct SkaRendererInitProps {
    int32 window_width;
    int32 window_height;
    int32 resolution_width;
    int32 resolution_height;
    bool maintain_aspect_ratio;
    bool init_2d;
    SkaRenderer3DInitProps* init_props_3d;
} SkaRendererInitProps;

void ska_renderer_init(SkaRendererInitProps props);
void ska_renderer_deinit();
void ska_renderer_clear(f32 red, f32 green, f32 blue, f32 alpha);
void ska_renderer_update_window_size(int32 window_width, int32 window_height);
void ska_renderer_set_sprite_shader_default_params(SkaShader* shader);

void ska_renderer_process_and_flush_batches(const SkaColor* background_color);
void ska_renderer_process_and_flush_batches_just_framebuffer(const SkaColor* background_color);

void ska_renderer_print_opengl_errors();

// Shader params
void ska_renderer_set_global_shader_param_time(f32 time_value);
void ska_renderer_set_shader_instance_params(SkaShaderInstance* shader_instance);

// 2D

void ska_renderer_init_2d();
void ska_renderer_deinit_2d();
void ska_renderer_queue_sprite_draw(SkaTexture* texture, SkaRect2 source_rect, SkaSize2D dest_size, SkaColor color, bool flip_h, bool flip_v, const SkaTransform2D* transform2_d, int32 z_index, SkaShaderInstance* shader_instance);
void ska_renderer_queue_sprite_draw2(SkaTexture* texture, SkaRect2 source_rect, SkaSize2D dest_size, SkaColor color, bool flip_h, bool flip_v, SkaMatrix4* trs_matrix, int32 z_index, SkaShaderInstance* shader_instance);
void ska_renderer_queue_font_draw_call(SkaFont* font, const char* text, f32 x, f32 y, f32 scale, SkaColor color, int32 z_index);
void ska_renderer_flush_2d_batches();

// 3D

typedef struct SkaDirectionalLightQueueProps {
    SkaVector3 direction;
    SkaVector3 ambient;
    SkaVector3 diffuse;
    SkaVector3 specular;
    int32 z_index;
    SkaShaderInstance* shader_instance;
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
    int32 z_index;
    SkaShaderInstance* shader_instance;
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
    bool is_attached_to_camera;
    float cutoff;
    float outer_cutoff;
    int32 z_index;
    SkaShaderInstance* shader_instance;
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
    f32 near_plane;
    f32 far_plane;
} SkaCamera3D;

#define SKA_DEFAULT_CAMERA_3D (SkaCamera3D) { .position = SKA_VECTOR3_ZERO, .rotation = SKA_VECTOR3_ZERO, .up = SKA_VECTOR3_UP, .front = SKA_VECTOR3_ZERO, .fov = 45.0f, .nearPlane = 0.1f, .farPlane = 1000.0f }

// Names the face being culled (matches OpenGL's own glCullFace semantics, not the face kept):
// SkaRenderCullMode_FRONT culls front faces, i.e. only back faces render. Used by techniques
// like inverted-hull outlines that need to draw an expanded duplicate of a mesh with only its
// back faces visible.
typedef enum SkaRenderCullMode {
    SkaRenderCullMode_NONE = 0,
    SkaRenderCullMode_BACK,
    SkaRenderCullMode_FRONT,
} SkaRenderCullMode;

// 3D Rendering functions
void ska_renderer_init_3d(SkaRenderer3DInitProps props);
void ska_renderer_set_view_matrix(const SkaMatrix4* view);
void ska_renderer_set_projection_matrix(const SkaMatrix4* projection);
void ska_renderer_queue_mesh_draw(SkaMesh* mesh, const SkaMatrix4* model, SkaShaderInstance* shader_instance);
void ska_renderer_queue_mesh_draw_ex(SkaMesh* mesh, const SkaMatrix4* model, SkaShaderInstance* shader_instance, SkaRenderCullMode cull_mode);
// Same as _ex, plus a color that multiplies the shader's lit result (see the mesh fragment
// shader's tintColor uniform) -- e.g. for a plain solid-color diffuse texture tinted per draw call,
// without needing a dedicated shader per color. _ex/queue_mesh_draw both forward here with an
// opaque-white (no-op) tint.
void ska_renderer_queue_mesh_draw_tinted(SkaMesh* mesh, const SkaMatrix4* model, SkaShaderInstance* shader_instance, SkaRenderCullMode cull_mode, SkaColor tint);
void ska_renderer_queue_directional_light_draw(SkaDirectionalLightQueueProps props);
void ska_renderer_queue_point_light_draw(SkaPointLightQueueProps props);
void ska_renderer_queue_spot_light_draw(SkaSpotLightQueueProps props);
void ska_renderer_draw_model(SkaModel* model, const SkaMatrix4* model_matrix, SkaShaderInstance* shader_instance);
void ska_renderer_flush_meshes();
void ska_renderer_flush_lights();

// Debug line batch: 'from'/'to' are world-space endpoints (no model matrix - bake any transform
// into the points yourself before queuing). Meant for gizmos/collider wireframes, not gameplay
// geometry - no depth-test override, so lines draw depth-tested like everything else in the scene.
void ska_renderer_queue_debug_line_draw(SkaVector3 from, SkaVector3 to, SkaColor color);
void ska_renderer_flush_debug_lines();

void ska_renderer_set_camera_position(SkaVector3 position);
// rotation is in degrees: x = pitch (tilt up/down), y = yaw (turn left/right). z (roll) is
// accepted but currently unused by the front-vector calculation. (0, 0, 0) looks down -Z, matching
// SkaCamera3D's default front vector.
void ska_renderer_set_camera_rotation(SkaVector3 rotation);
void ska_renderer_set_camera_fov(f32 fov_degrees);
SkaCamera3D* ska_renderer_get_active_camera();
void ska_renderer_get_camera_view_matrix(const SkaCamera3D* camera, SkaMatrix4* matrix4);
void ska_renderer_get_camera_projection_matrix(const SkaCamera3D* camera, SkaMatrix4* matrix4);

#ifdef __cplusplus
}
#endif

#endif // #if SKA_RENDERING
