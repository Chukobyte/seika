#if SKA_RENDERING

#include <math.h>
#include <stddef.h>

#include "renderer.h"
#include "render_context.h"
#include "seika/assert.h"
#include "../logger.h"
#include "shader/shader_source.h"

#define SKA_RENDER_POINT_LIGHT_MAX 16
#define SKA_RENDER_SPOT_LIGHT_MAX 16
#define SKA_RENDER_DIRECTIONAL_LIGHT_MAX 16
#define SKA_RENDER_MESH_MAX 64
#define SKA_RENDER_DEBUG_LINE_MAX 1024

typedef struct DirectionalLightBatchItem {
    SkaVector3 direction;
    SkaVector3 ambient;
    SkaVector3 diffuse;
    SkaVector3 specular;
} DirectionalLightBatchItem;

#define SKA_DEFAULT_DIRECTIONAL_LIGHT_BATCH_ITEM (DirectionalLightBatchItem){ .direction = { .x = -0.2f, .y = -1.0f, .z = -0.3f }, .ambient = { .x = 0.05f, .y = 0.05f, .z = 0.05f }, .diffuse = { .x = 0.4f, .y = 0.4f, .z = 0.4f }, .specular = { .x = 0.5f, .y = 0.5f, .z = 0.5f } }

typedef struct PointLightDrawBatchItem {
    SkaVector3 position;
    SkaVector3 scale;
    SkaVector3 ambient;
    SkaVector3 diffuse;
    SkaVector3 specular;
    float linear;
    float quadratic;
    float constant;
} PointLightDrawBatchItem;

#define SKA_DEFAULT_POINT_LIGHT_BATCH_ITEM (PointLightDrawBatchItem){ .position = { .x = 0.0f, .y = 0.0f, .z = 0.0f }, .scale = { .x = 0.2f, .y = 0.2f, .z = 0.2f }, .ambient = { .x = 0.05f, .y = 0.05f, .z = 0.05f }, .diffuse = { .x = 0.8f, .y = 0.8f, .z = 0.8f }, .specular = { .x = 1.0f, .y = 1.0f, .z = 1.0f }, .linear = 0.09f, .quadratic = 0.032f, .constant = 1.0f }

typedef struct SpotLightDrawBatchItem {
    SkaVector3 position;
    SkaVector3 direction;
    SkaVector3 ambient;
    SkaVector3 diffuse;
    SkaVector3 specular;
    float linear;
    float quadratic;
    float constant;
    bool is_attached_to_camera;
    // cutoff and outCutoff are in radians
    float cutoff;
    float outer_cutoff;
} SpotLightDrawBatchItem;

#define SKA_DEFAULT_SPOT_LIGHT_BATCH_ITEM (SpotLightDrawBatchItem){ .position = { .x = 0.0f, .y = 0.0f, .z = 0.0f }, .direction = { .x = 0.0f, .y = 0.0f, -1.0f }, .ambient = { .x = 0.05f, .y = 0.05f, .z = 0.05f }, .diffuse = { .x = 1.0f, .y = 1.0f, .z = 1.0f }, .specular = { .x = 1.0f, .y = 1.0f, .z = 1.0f }, .linear = 0.09f, .quadratic = 0.032f, .constant = 1.0f, .cutoff = 12.5f, .outerCutoff = 15.0f }

typedef struct MeshBatchItem {
    SkaMesh* mesh;
    SkaMatrix4 model_matrix;
    SkaShaderInstance* shader_instance;
    SkaRenderCullMode cull_mode;
    // tintColor uniform; opaque white (no-op) unless set via ska_renderer_queue_mesh_draw_tinted.
    SkaColor tint;
} MeshBatchItem;

// Position + color interleaved, matching the debug-line vertex shader's two attributes.
typedef struct DebugLineVertex {
    SkaVector3 position;
    SkaColor color;
} DebugLineVertex;

typedef struct LightBatchItems {
    DirectionalLightBatchItem directional_lights[SKA_RENDER_DIRECTIONAL_LIGHT_MAX];
    PointLightDrawBatchItem point_lights[SKA_RENDER_POINT_LIGHT_MAX];
    SpotLightDrawBatchItem spot_lights[SKA_RENDER_SPOT_LIGHT_MAX];
    usize directional_light_count;
    usize point_light_count;
    usize spot_light_count;
} LightBatchItems;

static void renderer_batching_draw_meshes(MeshBatchItem items[], usize mesh_count);

static GLuint point_light_vao;
static GLuint point_light_vbo;
static SkaShader* mesh_shader = NULL;
static SkaShader* point_light_shader = NULL;
static SkaMatrix4 view_matrix = {0};
static SkaMatrix4 projection_matrix = {0};
static SkaCamera3D current_camera3_d = {0};
static LightBatchItems light_batch_items = {0};
static MeshBatchItem mesh_batch_items[SKA_RENDER_MESH_MAX] = {{0}};
static usize mesh_batch_item_count = 0;

static GLuint debug_line_vao;
static GLuint debug_line_vbo;
static SkaShader* debug_line_shader = NULL;
static DebugLineVertex debug_line_batch_items[SKA_RENDER_DEBUG_LINE_MAX * 2] = {{0}};
static usize debug_line_batch_vertex_count = 0;

// --- Mesh Renderer --- //

// --- 3D Renderer Initialization --- //
void ska_renderer_init_3d(SkaRenderer3DInitProps props) {
// void ska_renderer_init_3d(float fov, float nearPlane, float farPlane) {
    // Compile 3D shaders
    // meshShader = ska_shader_compile_new_shader(ska_opengl_shader_source_vertex_mesh, ska_opengl_shader_source_fragment_mesh);
    mesh_shader = ska_shader_compile_new_shader(ska_opengl_shader_source_vertex_mesh, ska_opengl_shader_source_fragment_mesh);
    point_light_shader = ska_shader_compile_new_shader(opengl_shader_source_point_light_vertex, opengl_shader_source_point_light_fragment);
    debug_line_shader = ska_shader_compile_new_shader(opengl_shader_source_debug_line_vertex, opengl_shader_source_debug_line_fragment);
    SKA_ASSERT(mesh_shader);
    SKA_ASSERT(point_light_shader);
    SKA_ASSERT(debug_line_shader);

    // Setup point light stuff
    glGenVertexArrays(1, &point_light_vao);
    glBindVertexArray(point_light_vao);
    glGenBuffers(1, &point_light_vbo);

    // TODO: Figure out if we'll need to buffer the data afterwards
    glBindBuffer(GL_ARRAY_BUFFER, point_light_vbo);
    // Vertices for a cube (commonly used for point light visualization)
    const GLfloat vertices[] = {
        // positions
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 0.0f, 0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 0.0f, 0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 0.0f, 0.0f, 0.0f,

         0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 0.0f, 0.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 0.0f, 0.0f, 0.0f,

        -0.5f,  0.5f, -0.5f,  0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 0.0f, 0.0f, 0.0f, 0.0f
    };
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)NULL);
    glEnableVertexAttribArray(0);

    // Dynamic VBO, re-uploaded whole each flush - line volume per frame is too low to need better.
    glGenVertexArrays(1, &debug_line_vao);
    glBindVertexArray(debug_line_vao);
    glGenBuffers(1, &debug_line_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, debug_line_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(debug_line_batch_items), NULL, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(DebugLineVertex), (void*)offsetof(DebugLineVertex, position));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(DebugLineVertex), (void*)offsetof(DebugLineVertex, color));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    // Setup projection matrix
    const SkaSize2Di resolution = ska_render_context_get()->resolution;
    const f32 aspect_ratio = (f32)resolution.w / (f32)resolution.h;
    projection_matrix = ska_matrix4_perspective(
        SKA_DEG_2_RADF(props.fov),
        aspect_ratio,
        props.near_plane,
        props.far_plane
    );

    current_camera3_d.position.x = 0.0f;
    current_camera3_d.position.y = 0.0f;
    current_camera3_d.position.z = 0.0f;
    current_camera3_d.rotation.x = 0.0f;
    current_camera3_d.rotation.y = 0.0f;
    current_camera3_d.rotation.z = 0.0f;
    current_camera3_d.up.x = 0.0f;
    current_camera3_d.up.y = 1.0f;
    current_camera3_d.up.z = 0.0f;
    // Must be non-zero (look_at degenerates when front==0) and match set_camera_rotation(0,0)'s output.
    current_camera3_d.front.x = 0.0f;
    current_camera3_d.front.y = 0.0f;
    current_camera3_d.front.z = -1.0f;

    current_camera3_d.fov = props.fov;
    current_camera3_d.near_plane = props.near_plane;
    current_camera3_d.far_plane = props.far_plane;

    // Enable depth testing for 3D
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
}

void ska_renderer_set_view_matrix(const SkaMatrix4* view) {
    ska_matrix4_copy_implace(&view_matrix, view);
}

void ska_renderer_set_projection_matrix(const SkaMatrix4* projection) {
    ska_matrix4_copy_implace(&projection_matrix, projection);
}

void ska_renderer_queue_mesh_draw(SkaMesh* mesh, const SkaMatrix4* model, SkaShaderInstance* shader_instance) {
    ska_renderer_queue_mesh_draw_ex(mesh, model, shader_instance, SkaRenderCullMode_NONE);
}

void ska_renderer_queue_mesh_draw_ex(SkaMesh* mesh, const SkaMatrix4* model, SkaShaderInstance* shader_instance, SkaRenderCullMode cull_mode) {
    ska_renderer_queue_mesh_draw_tinted(mesh, model, shader_instance, cull_mode, (SkaColor){ 1.0f, 1.0f, 1.0f, 1.0f });
}

void ska_renderer_queue_mesh_draw_tinted(SkaMesh* mesh, const SkaMatrix4* model, SkaShaderInstance* shader_instance, SkaRenderCullMode cull_mode, SkaColor tint) {
    if (mesh == NULL) {
        ska_logger_error("NULL mesh, not submitting draw call!");
        return;
    }

    // const int32 arrayZIndex = ska_math_clamp_int(zIndex + SKA_RENDERER_MAX_Z_INDEX / 2, 0, SKA_RENDERER_MAX_Z_INDEX - 1);

    MeshBatchItem item;
    item.mesh = mesh;
    item.model_matrix = SKA_MATRIX4_IDENTITY;
    item.shader_instance = shader_instance;
    item.cull_mode = cull_mode;
    item.tint = tint;
    ska_matrix4_copy_implace(&item.model_matrix, model);

    SKA_ASSERT(mesh_batch_item_count < SKA_RENDER_MESH_MAX);
    mesh_batch_items[mesh_batch_item_count++] = item;
}

void ska_renderer_queue_directional_light_draw(SkaDirectionalLightQueueProps props) {
    SKA_ASSERT(light_batch_items.directional_light_count < SKA_RENDER_DIRECTIONAL_LIGHT_MAX);
    DirectionalLightBatchItem item;
    item.direction = props.direction;
    item.ambient = props.ambient;
    item.diffuse = props.diffuse;
    item.specular = props.specular;
    light_batch_items.directional_lights[light_batch_items.directional_light_count++] = item;
}

void ska_renderer_queue_point_light_draw(SkaPointLightQueueProps props) {
    SKA_ASSERT(light_batch_items.point_light_count < SKA_RENDER_POINT_LIGHT_MAX);
    PointLightDrawBatchItem item = {
        .position = props.position,
        .scale = props.scale,
        .ambient = props.ambient,
        .diffuse = props.diffuse,
        .specular = props.specular,
        .linear = props.linear,
        .quadratic = props.quadratic,
        .constant = props.constant
    };
    light_batch_items.point_lights[light_batch_items.point_light_count++] = item;
}

void ska_renderer_queue_spot_light_draw(SkaSpotLightQueueProps props) {
    SKA_ASSERT(light_batch_items.spot_light_count < SKA_RENDER_SPOT_LIGHT_MAX);
    const SpotLightDrawBatchItem item = {
        .position = props.position,
        .direction = props.direction,
        .ambient = props.ambient,
        .diffuse = props.diffuse,
        .specular = props.specular,
        .linear = props.linear,
        .quadratic = props.quadratic,
        .constant = props.constant,
        .is_attached_to_camera = props.is_attached_to_camera,
        .cutoff = props.cutoff,
        .outer_cutoff = props.outer_cutoff
    };
    light_batch_items.spot_lights[light_batch_items.spot_light_count++] = item;
}

void ska_renderer_queue_debug_line_draw(SkaVector3 from, SkaVector3 to, SkaColor color) {
    if (debug_line_batch_vertex_count + 2 > SKA_RENDER_DEBUG_LINE_MAX * 2) {
        ska_logger_error("Debug line batch full, not submitting line draw!");
        return;
    }
    debug_line_batch_items[debug_line_batch_vertex_count++] = (DebugLineVertex){ .position = from, .color = color };
    debug_line_batch_items[debug_line_batch_vertex_count++] = (DebugLineVertex){ .position = to, .color = color };
}

void renderer_batching_draw_meshes(MeshBatchItem items[], usize mesh_count) {
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);

    for (usize i = 0; i < mesh_count; i++) {
        MeshBatchItem* item = &items[i];

        // Set explicitly every draw since items in the same batch can have different cull modes.
        if (item->cull_mode == SkaRenderCullMode_NONE) {
            glDisable(GL_CULL_FACE);
        } else {
            glEnable(GL_CULL_FACE);
            glCullFace(item->cull_mode == SkaRenderCullMode_FRONT ? GL_FRONT : GL_BACK);
        }

        // Use shader
        // TODO: Can group setting shader uniform when they have the same instance
        SkaShader* shader = item->shader_instance ? item->shader_instance->shader : mesh_shader;
        ska_shader_use(shader);

        // Set matrices
        ska_shader_set_mat4_float(shader, "model", &item->model_matrix);
        ska_shader_set_mat4_float(shader, "view", &view_matrix);
        ska_shader_set_mat4_float(shader, "projection", &projection_matrix);
        ska_shader_set_vec3_float(shader, "viewPos", current_camera3_d.position.x, current_camera3_d.position.y, current_camera3_d.position.z);
        ska_shader_set_int(shader, "numberOfDirectionalLights", (int32)light_batch_items.directional_light_count);
        ska_shader_set_int(shader, "numberOfPointLights", (int32)light_batch_items.point_light_count);
        ska_shader_set_int(shader, "numberOfSpotLights", (int32)light_batch_items.spot_light_count);
        ska_shader_set_int(shader, "material.diffuse", 0);
        ska_shader_set_int(shader, "material.specular", 1);
        ska_shader_set_float(shader, "material.shininess", 0.8f);
        // Harmless no-op on custom shader_instances: unknown uniform locations are silently ignored.
        ska_shader_set_vec4_float(shader, "tintColor", item->tint.r, item->tint.g, item->tint.b, item->tint.a);

        // Lights
        // TODO: Can batch lights once we make sure all meshes in batch have the same shader instance

        // Directional Lights
        for (usize dir_idx = 0; dir_idx < light_batch_items.directional_light_count; dir_idx++) {
            char uniform_name[128];
            const DirectionalLightBatchItem* dir_light = &light_batch_items.directional_lights[dir_idx];

            snprintf(uniform_name, sizeof(uniform_name), "directionalLights[%zu].direction", dir_idx);
            ska_shader_set_vec3_float(shader, uniform_name, dir_light->direction.x, dir_light->direction.y, dir_light->direction.z);

            snprintf(uniform_name, sizeof(uniform_name), "directionalLights[%zu].ambient", dir_idx);
            ska_shader_set_vec3_float(shader, uniform_name, dir_light->ambient.x, dir_light->ambient.y, dir_light->ambient.z);

            snprintf(uniform_name, sizeof(uniform_name), "directionalLights[%zu].diffuse", dir_idx);
            ska_shader_set_vec3_float(shader, uniform_name, dir_light->diffuse.x, dir_light->diffuse.y, dir_light->diffuse.z);

            snprintf(uniform_name, sizeof(uniform_name), "directionalLights[%zu].specular", dir_idx);
            ska_shader_set_vec3_float(shader, uniform_name, dir_light->specular.x, dir_light->specular.y, dir_light->specular.z);
        }

        // Point Lights
        for (usize point_idx = 0; point_idx < light_batch_items.point_light_count; point_idx++) {
            char uniform_name[128];
            const PointLightDrawBatchItem* point_light = &light_batch_items.point_lights[point_idx];

            snprintf(uniform_name, sizeof(uniform_name), "pointLights[%zu].position", point_idx);
            ska_shader_set_vec3_float(shader, uniform_name, point_light->position.x, point_light->position.y, point_light->position.z);

            snprintf(uniform_name, sizeof(uniform_name), "pointLights[%zu].ambient", point_idx);
            ska_shader_set_vec3_float(shader, uniform_name, point_light->ambient.x, point_light->ambient.y, point_light->ambient.z);

            snprintf(uniform_name, sizeof(uniform_name), "pointLights[%zu].diffuse", point_idx);
            ska_shader_set_vec3_float(shader, uniform_name, point_light->diffuse.x, point_light->diffuse.y, point_light->diffuse.z);

            snprintf(uniform_name, sizeof(uniform_name), "pointLights[%zu].specular", point_idx);
            ska_shader_set_vec3_float(shader, uniform_name, point_light->specular.x, point_light->specular.y, point_light->specular.z);

            snprintf(uniform_name, sizeof(uniform_name), "pointLights[%zu].constant", point_idx);
            ska_shader_set_float(shader, uniform_name, point_light->constant);

            snprintf(uniform_name, sizeof(uniform_name), "pointLights[%zu].linear", point_idx);
            ska_shader_set_float(shader, uniform_name, point_light->linear);

            snprintf(uniform_name, sizeof(uniform_name), "pointLights[%zu].quadratic", point_idx);
            ska_shader_set_float(shader, uniform_name, point_light->quadratic);
        }

        // Spot Lights
        for (usize spot_idx = 0; spot_idx < light_batch_items.spot_light_count; spot_idx++) {
            char uniform_name[128];
            const SpotLightDrawBatchItem* spot_light = &light_batch_items.spot_lights[spot_idx];
            SkaVector3 spot_light_position = spot_light->position;
            SkaVector3 spot_light_direction = spot_light->direction;

            // TODO: temp work around, implement in system
            if (spot_light->is_attached_to_camera) {
                spot_light_position = current_camera3_d.position;
                spot_light_direction = current_camera3_d.front;
            }

            snprintf(uniform_name, sizeof(uniform_name), "spotLights[%zu].position", spot_idx);
            ska_shader_set_vec3_float(shader, uniform_name, spot_light_position.x, spot_light_position.y, spot_light_position.z);

            snprintf(uniform_name, sizeof(uniform_name), "spotLights[%zu].direction", spot_idx);
            ska_shader_set_vec3_float(shader, uniform_name, spot_light_direction.x, spot_light_direction.y, spot_light_direction.z);

            snprintf(uniform_name, sizeof(uniform_name), "spotLights[%zu].ambient", spot_idx);
            ska_shader_set_vec3_float(shader, uniform_name, spot_light->ambient.x, spot_light->ambient.y, spot_light->ambient.z);

            snprintf(uniform_name, sizeof(uniform_name), "spotLights[%zu].diffuse", spot_idx);
            ska_shader_set_vec3_float(shader, uniform_name, spot_light->diffuse.x, spot_light->diffuse.y, spot_light->diffuse.z);

            snprintf(uniform_name, sizeof(uniform_name), "spotLights[%zu].specular", spot_idx);
            ska_shader_set_vec3_float(shader, uniform_name, spot_light->specular.x, spot_light->specular.y, spot_light->specular.z);

            snprintf(uniform_name, sizeof(uniform_name), "spotLights[%zu].constant", spot_idx);
            ska_shader_set_float(shader, uniform_name, spot_light->constant);

            snprintf(uniform_name, sizeof(uniform_name), "spotLights[%zu].linear", spot_idx);
            ska_shader_set_float(shader, uniform_name, spot_light->linear);

            snprintf(uniform_name, sizeof(uniform_name), "spotLights[%zu].quadratic", spot_idx);
            ska_shader_set_float(shader, uniform_name, spot_light->quadratic);

            snprintf(uniform_name, sizeof(uniform_name), "spotLights[%zu].cutOff", spot_idx);
            ska_shader_set_float(shader, uniform_name, spot_light->cutoff);

            snprintf(uniform_name, sizeof(uniform_name), "spotLights[%zu].outerCutOff", spot_idx);
            ska_shader_set_float(shader, uniform_name, spot_light->outer_cutoff);
        }

        // Set shader instance params if available
        if (item->shader_instance) {
            ska_renderer_set_shader_instance_params(item->shader_instance);
        }

        // bind diffuse map
        if (item->mesh->diffuse) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, item->mesh->diffuse->id);
        }
        // bind specular map
        if (item->mesh->specular) {
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, item->mesh->specular->id);
        }
        // bind normal map
        if (item->mesh->normal) {
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, item->mesh->normal->id);
        }

        // Draw mesh
        glBindVertexArray(item->mesh->vao);
        glBindBuffer(GL_ARRAY_BUFFER, item->mesh->vbo);

        if (item->mesh->has_indices) {
          glDrawElements(GL_TRIANGLES, (GLsizei)item->mesh->index_count,
                         GL_UNSIGNED_INT, 0);
        } else {
          glDrawArrays(GL_TRIANGLES, 0, (GLsizei)item->mesh->vertex_count);
        }
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    ska_renderer_print_opengl_errors();
}

void renderer_draw_queued_pointLights() {
    ska_shader_use(point_light_shader);
    ska_shader_set_mat4_float(point_light_shader, "projection", &projection_matrix);
    ska_shader_set_mat4_float(point_light_shader, "view", &view_matrix);

    glBindVertexArray(point_light_vao);
    for (usize i = 0; i < light_batch_items.point_light_count; i++) {
        const PointLightDrawBatchItem* item = &light_batch_items.point_lights[i];
        SkaMatrix4 model = SKA_MATRIX4_IDENTITY;
        ska_matrix4_translate(&model, item->position);
        ska_matrix4_scale(&model, item->scale);
        ska_shader_set_mat4_float(point_light_shader, "model", &model);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
}

void ska_renderer_set_camera_position(SkaVector3 position) {
    current_camera3_d.position = position;
}

void ska_renderer_set_camera_rotation(SkaVector3 rotation) {
    current_camera3_d.rotation = rotation;
    // Yaw measured from -Z (not +X) so (0,0,0) matches the default front vector; already unit length.
    const f32 yaw_rad = SKA_DEG_2_RADF(rotation.y);
    const f32 pitch_rad = SKA_DEG_2_RADF(rotation.x);
    current_camera3_d.front.x = sinf(yaw_rad) * cosf(pitch_rad);
    current_camera3_d.front.y = sinf(pitch_rad);
    current_camera3_d.front.z = -cosf(yaw_rad) * cosf(pitch_rad);
}

void ska_renderer_set_camera_fov(f32 fov_degrees) {
    current_camera3_d.fov = fov_degrees;
}

SkaCamera3D* ska_renderer_get_active_camera() {
    return &current_camera3_d;
}

void ska_renderer_get_camera_view_matrix(const SkaCamera3D* camera, SkaMatrix4* matrix4) {
    *matrix4 = ska_matrix4_look_at(
        camera->position,
        ska_vector3_add(camera->position, camera->front),
        camera->up
    );
}

void ska_renderer_get_camera_projection_matrix(const SkaCamera3D* camera, SkaMatrix4* matrix4) {
    const SkaSize2Di resolution = ska_render_context_get()->resolution;
    const f32 aspect_ratio = (f32)resolution.w / (f32)resolution.h;
    *matrix4 = ska_matrix4_perspective(SKA_DEG_2_RADF(camera->fov), aspect_ratio, camera->near_plane, camera->far_plane);
}

void ska_renderer_flush_meshes() {
    // ska_renderer_get_camera_projection_matrix(ska_renderer_get_active_camera(), &projectionMatrix);
    // ska_renderer_get_camera_view_matrix(ska_renderer_get_active_camera(), &viewMatrix);
    if (mesh_batch_item_count == 0) return;

    renderer_batching_draw_meshes(mesh_batch_items, mesh_batch_item_count);
    mesh_batch_item_count = 0;
}

void ska_renderer_flush_lights() {
    renderer_draw_queued_pointLights();
    light_batch_items.directional_light_count = 0;
    light_batch_items.point_light_count = 0;
    light_batch_items.spot_light_count = 0;
}

void ska_renderer_flush_debug_lines() {
    if (debug_line_batch_vertex_count == 0) {
        return;
    }

    ska_shader_use(debug_line_shader);
    ska_shader_set_mat4_float(debug_line_shader, "view", &view_matrix);
    ska_shader_set_mat4_float(debug_line_shader, "projection", &projection_matrix);

    glBindVertexArray(debug_line_vao);
    glBindBuffer(GL_ARRAY_BUFFER, debug_line_vbo);
    // glBufferSubData, not glBufferData: VBO already sized to max at init, no need to reallocate.
    glBufferSubData(GL_ARRAY_BUFFER, 0, (GLsizeiptr)(debug_line_batch_vertex_count * sizeof(DebugLineVertex)), debug_line_batch_items);
    glDrawArrays(GL_LINES, 0, (GLsizei)debug_line_batch_vertex_count);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    debug_line_batch_vertex_count = 0;
}

#endif // #if SKA_RENDERING
