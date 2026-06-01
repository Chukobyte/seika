#if SKA_RENDERING

#include "renderer.h"
#include "render_context.h"
#include "seika/assert.h"
#include "../logger.h"
#include "shader/shader_source.h"

#define SKA_RENDER_POINT_LIGHT_MAX 16
#define SKA_RENDER_SPOT_LIGHT_MAX 16
#define SKA_RENDER_DIRECTIONAL_LIGHT_MAX 16
#define SKA_RENDER_MESH_MAX 64

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
    bool isAttachedToCamera;
    // cutoff and outCutoff are in radians
    float cutoff;
    float outerCutoff;
} SpotLightDrawBatchItem;

#define SKA_DEFAULT_SPOT_LIGHT_BATCH_ITEM (SpotLightDrawBatchItem){ .position = { .x = 0.0f, .y = 0.0f, .z = 0.0f }, .direction = { .x = 0.0f, .y = 0.0f, -1.0f }, .ambient = { .x = 0.05f, .y = 0.05f, .z = 0.05f }, .diffuse = { .x = 1.0f, .y = 1.0f, .z = 1.0f }, .specular = { .x = 1.0f, .y = 1.0f, .z = 1.0f }, .linear = 0.09f, .quadratic = 0.032f, .constant = 1.0f, .cutoff = 12.5f, .outerCutoff = 15.0f }

typedef struct MeshBatchItem {
    SkaMesh* mesh;
    SkaMatrix4 modelMatrix;
    SkaShaderInstance* shaderInstance;
} MeshBatchItem;

typedef struct LightBatchItems {
    DirectionalLightBatchItem directionalLights[SKA_RENDER_DIRECTIONAL_LIGHT_MAX];
    PointLightDrawBatchItem pointLights[SKA_RENDER_POINT_LIGHT_MAX];
    SpotLightDrawBatchItem spotLights[SKA_RENDER_SPOT_LIGHT_MAX];
    usize directionalLightCount;
    usize pointLightCount;
    usize spotLightCount;
} LightBatchItems;

static void renderer_batching_draw_meshes(MeshBatchItem items[], usize meshCount);

static GLuint pointLightVAO;
static GLuint pointLightVBO;
static SkaShader* meshShader = NULL;
static SkaShader* pointLightShader = NULL;
static SkaMatrix4 viewMatrix = {0};
static SkaMatrix4 projectionMatrix = {0};
static SkaCamera3D currentCamera3D = {0};
static LightBatchItems lightBatchItems = {0};
static MeshBatchItem meshBatchItems[SKA_RENDER_MESH_MAX] = {{0}};
static usize meshBatchItemCount = 0;

// --- Mesh Renderer --- //

// --- 3D Renderer Initialization --- //
void ska_renderer_init_3d(SkaRenderer3DInitProps props) {
// void ska_renderer_init_3d(float fov, float nearPlane, float farPlane) {
    // Compile 3D shaders
    // meshShader = ska_shader_compile_new_shader(SKA_OPENGL_SHADER_SOURCE_VERTEX_MESH, SKA_OPENGL_SHADER_SOURCE_FRAGMENT_MESH);
    meshShader = ska_shader_compile_new_shader(SKA_OPENGL_SHADER_SOURCE_VERTEX_MESH, SKA_OPENGL_SHADER_SOURCE_FRAGMENT_MESH);
    pointLightShader = ska_shader_compile_new_shader(OPENGL_SHADER_SOURCE_POINT_LIGHT_VERTEX, OPENGL_SHADER_SOURCE_POINT_LIGHT_FRAGMENT);
    SKA_ASSERT(meshShader);
    SKA_ASSERT(pointLightShader);

    // Setup point light stuff
    glGenVertexArrays(1, &pointLightVAO);
    glBindVertexArray(pointLightVAO);
    glGenBuffers(1, &pointLightVBO);

    // TODO: Figure out if we'll need to buffer the data afterwards
    glBindBuffer(GL_ARRAY_BUFFER, pointLightVBO);
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

    // Setup projection matrix
    const SkaSize2Di resolution = ska_render_context_get()->resolution;
    const f32 aspectRatio = (f32)resolution.w / (f32)resolution.h;
    projectionMatrix = ska_matrix4_perspective(
        SKA_DEG_2_RAD(props.fov),
        aspectRatio,
        props.nearPlane,
        props.farPlane
    );

    currentCamera3D.position.x = 0.0f;
    currentCamera3D.position.y = 0.0f;
    currentCamera3D.position.z = 0.0f;
    currentCamera3D.rotation.x = 0.0f;
    currentCamera3D.rotation.y = 0.0f;
    currentCamera3D.rotation.z = 0.0f;
    currentCamera3D.up.x = 0.0f;
    currentCamera3D.up.y = 1.0f;
    currentCamera3D.up.z = 0.0f;
    currentCamera3D.front.x = 0.0f;
    currentCamera3D.front.y = 0.0f;
    currentCamera3D.front.z = 0.0f;

    currentCamera3D.fov = props.fov;
    currentCamera3D.nearPlane = props.nearPlane;
    currentCamera3D.farPlane = props.farPlane;

    // Enable depth testing for 3D
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
}

void ska_renderer_set_view_matrix(const SkaMatrix4* view) {
    ska_matrix4_copy_implace(&viewMatrix, view);
}

void ska_renderer_set_projection_matrix(const SkaMatrix4* projection) {
    ska_matrix4_copy_implace(&projectionMatrix, projection);
}

void ska_renderer_queue_mesh_draw(SkaMesh* mesh, const SkaMatrix4* model, SkaShaderInstance* shaderInstance) {
    if (mesh == NULL) {
        ska_logger_error("NULL mesh, not submitting draw call!");
        return;
    }

    // const int32 arrayZIndex = ska_math_clamp_int(zIndex + SKA_RENDERER_MAX_Z_INDEX / 2, 0, SKA_RENDERER_MAX_Z_INDEX - 1);

    MeshBatchItem item;
    item.mesh = mesh;
    item.modelMatrix = SKA_MATRIX4_IDENTITY;
    item.shaderInstance = shaderInstance;
    ska_matrix4_copy_implace(&item.modelMatrix, model);

    SKA_ASSERT(meshBatchItemCount < SKA_RENDER_MESH_MAX);
    meshBatchItems[meshBatchItemCount++] = item;
}

void ska_renderer_queue_directional_light_draw(SkaDirectionalLightQueueProps props) {
    SKA_ASSERT(lightBatchItems.directionalLightCount < SKA_RENDER_DIRECTIONAL_LIGHT_MAX);
    DirectionalLightBatchItem item;
    item.direction = props.direction;
    item.ambient = props.ambient;
    item.diffuse = props.diffuse;
    item.specular = props.specular;
    lightBatchItems.directionalLights[lightBatchItems.directionalLightCount++] = item;
}

void ska_renderer_queue_point_light_draw(SkaPointLightQueueProps props) {
    SKA_ASSERT(lightBatchItems.pointLightCount < SKA_RENDER_POINT_LIGHT_MAX);
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
    lightBatchItems.pointLights[lightBatchItems.pointLightCount++] = item;
}

void ska_renderer_queue_spot_light_draw(SkaSpotLightQueueProps props) {
    SKA_ASSERT(lightBatchItems.spotLightCount < SKA_RENDER_SPOT_LIGHT_MAX);
    const SpotLightDrawBatchItem item = {
        .position = props.position,
        .direction = props.direction,
        .ambient = props.ambient,
        .diffuse = props.diffuse,
        .specular = props.specular,
        .linear = props.linear,
        .quadratic = props.quadratic,
        .constant = props.constant,
        .isAttachedToCamera = props.isAttachedToCamera,
        .cutoff = props.cutoff,
        .outerCutoff = props.outerCutoff
    };
    lightBatchItems.spotLights[lightBatchItems.spotLightCount++] = item;
}

void renderer_batching_draw_meshes(MeshBatchItem items[], usize meshCount) {
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);

    for (usize i = 0; i < meshCount; i++) {
        MeshBatchItem* item = &items[i];

        // Use shader
        // TODO: Can group setting shader uniform when they have the same instance
        SkaShader* shader = item->shaderInstance ? item->shaderInstance->shader : meshShader;
        ska_shader_use(shader);

        // Set matrices
        ska_shader_set_mat4_float(shader, "model", &item->modelMatrix);
        ska_shader_set_mat4_float(shader, "view", &viewMatrix);
        ska_shader_set_mat4_float(shader, "projection", &projectionMatrix);
        ska_shader_set_vec3_float(shader, "viewPos", currentCamera3D.position.x, currentCamera3D.position.y, currentCamera3D.position.z);
        ska_shader_set_int(shader, "numberOfDirectionalLights", (int32)lightBatchItems.directionalLightCount);
        ska_shader_set_int(shader, "numberOfPointLights", (int32)lightBatchItems.pointLightCount);
        ska_shader_set_int(shader, "numberOfSpotLights", (int32)lightBatchItems.spotLightCount);
        ska_shader_set_int(shader, "material.diffuse", 0);
        ska_shader_set_int(shader, "material.specular", 1);
        ska_shader_set_float(shader, "material.shininess", 0.8f);

        // Lights
        // TODO: Can batch lights once we make sure all meshes in batch have the same shader instance

        // Directional Lights
        for (usize dirIdx = 0; dirIdx < lightBatchItems.directionalLightCount; dirIdx++) {
            char uniformName[128];
            const DirectionalLightBatchItem* dirLight = &lightBatchItems.directionalLights[dirIdx];

            snprintf(uniformName, sizeof(uniformName), "directionalLights[%zu].direction", dirIdx);
            ska_shader_set_vec3_float(shader, uniformName, dirLight->direction.x, dirLight->direction.y, dirLight->direction.z);

            snprintf(uniformName, sizeof(uniformName), "directionalLights[%zu].ambient", dirIdx);
            ska_shader_set_vec3_float(shader, uniformName, dirLight->ambient.x, dirLight->ambient.y, dirLight->ambient.z);

            snprintf(uniformName, sizeof(uniformName), "directionalLights[%zu].diffuse", dirIdx);
            ska_shader_set_vec3_float(shader, uniformName, dirLight->diffuse.x, dirLight->diffuse.y, dirLight->diffuse.z);

            snprintf(uniformName, sizeof(uniformName), "directionalLights[%zu].specular", dirIdx);
            ska_shader_set_vec3_float(shader, uniformName, dirLight->specular.x, dirLight->specular.y, dirLight->specular.z);
        }

        // Point Lights
        for (usize pointIdx = 0; pointIdx < lightBatchItems.pointLightCount; pointIdx++) {
            char uniformName[128];
            const PointLightDrawBatchItem* pointLight = &lightBatchItems.pointLights[pointIdx];

            snprintf(uniformName, sizeof(uniformName), "pointLights[%zu].position", pointIdx);
            ska_shader_set_vec3_float(shader, uniformName, pointLight->position.x, pointLight->position.y, pointLight->position.z);

            snprintf(uniformName, sizeof(uniformName), "pointLights[%zu].ambient", pointIdx);
            ska_shader_set_vec3_float(shader, uniformName, pointLight->ambient.x, pointLight->ambient.y, pointLight->ambient.z);

            snprintf(uniformName, sizeof(uniformName), "pointLights[%zu].diffuse", pointIdx);
            ska_shader_set_vec3_float(shader, uniformName, pointLight->diffuse.x, pointLight->diffuse.y, pointLight->diffuse.z);

            snprintf(uniformName, sizeof(uniformName), "pointLights[%zu].specular", pointIdx);
            ska_shader_set_vec3_float(shader, uniformName, pointLight->specular.x, pointLight->specular.y, pointLight->specular.z);

            snprintf(uniformName, sizeof(uniformName), "pointLights[%zu].constant", pointIdx);
            ska_shader_set_float(shader, uniformName, pointLight->constant);

            snprintf(uniformName, sizeof(uniformName), "pointLights[%zu].linear", pointIdx);
            ska_shader_set_float(shader, uniformName, pointLight->linear);

            snprintf(uniformName, sizeof(uniformName), "pointLights[%zu].quadratic", pointIdx);
            ska_shader_set_float(shader, uniformName, pointLight->quadratic);
        }

        // Spot Lights
        for (usize spotIdx = 0; spotIdx < lightBatchItems.spotLightCount; spotIdx++) {
            char uniformName[128];
            const SpotLightDrawBatchItem* spotLight = &lightBatchItems.spotLights[spotIdx];
            SkaVector3 spotLightPosition = spotLight->position;
            SkaVector3 spotLightDirection = spotLight->direction;

            // TODO: temp work around, implement in system
            if (spotLight->isAttachedToCamera) {
                spotLightPosition = currentCamera3D.position;
                spotLightDirection = currentCamera3D.front;
            }

            snprintf(uniformName, sizeof(uniformName), "spotLights[%zu].position", spotIdx);
            ska_shader_set_vec3_float(shader, uniformName, spotLightPosition.x, spotLightPosition.y, spotLightPosition.z);

            snprintf(uniformName, sizeof(uniformName), "spotLights[%zu].direction", spotIdx);
            ska_shader_set_vec3_float(shader, uniformName, spotLightDirection.x, spotLightDirection.y, spotLightDirection.z);

            snprintf(uniformName, sizeof(uniformName), "spotLights[%zu].ambient", spotIdx);
            ska_shader_set_vec3_float(shader, uniformName, spotLight->ambient.x, spotLight->ambient.y, spotLight->ambient.z);

            snprintf(uniformName, sizeof(uniformName), "spotLights[%zu].diffuse", spotIdx);
            ska_shader_set_vec3_float(shader, uniformName, spotLight->diffuse.x, spotLight->diffuse.y, spotLight->diffuse.z);

            snprintf(uniformName, sizeof(uniformName), "spotLights[%zu].specular", spotIdx);
            ska_shader_set_vec3_float(shader, uniformName, spotLight->specular.x, spotLight->specular.y, spotLight->specular.z);

            snprintf(uniformName, sizeof(uniformName), "spotLights[%zu].constant", spotIdx);
            ska_shader_set_float(shader, uniformName, spotLight->constant);

            snprintf(uniformName, sizeof(uniformName), "spotLights[%zu].linear", spotIdx);
            ska_shader_set_float(shader, uniformName, spotLight->linear);

            snprintf(uniformName, sizeof(uniformName), "spotLights[%zu].quadratic", spotIdx);
            ska_shader_set_float(shader, uniformName, spotLight->quadratic);

            snprintf(uniformName, sizeof(uniformName), "spotLights[%zu].cutOff", spotIdx);
            ska_shader_set_float(shader, uniformName, spotLight->cutoff);

            snprintf(uniformName, sizeof(uniformName), "spotLights[%zu].outerCutOff", spotIdx);
            ska_shader_set_float(shader, uniformName, spotLight->outerCutoff);
        }

        // Set shader instance params if available
        if (item->shaderInstance) {
            ska_renderer_set_shader_instance_params(item->shaderInstance);
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

        if (item->mesh->hasIndices) {
            glDrawElements(GL_TRIANGLES, (GLsizei)item->mesh->indexCount, GL_UNSIGNED_INT, 0);
        } else {
            glDrawArrays(GL_TRIANGLES, 0, (GLsizei)item->mesh->vertexCount);
        }
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    ska_renderer_print_opengl_errors();
}

void renderer_draw_queued_pointLights() {
    ska_shader_use(pointLightShader);
    ska_shader_set_mat4_float(pointLightShader, "projection", &projectionMatrix);
    ska_shader_set_mat4_float(pointLightShader, "view", &viewMatrix);

    glBindVertexArray(pointLightVAO);
    for (usize i = 0; i < lightBatchItems.pointLightCount; i++) {
        const PointLightDrawBatchItem* item = &lightBatchItems.pointLights[i];
        SkaMatrix4 model = SKA_MATRIX4_IDENTITY;
        ska_matrix4_translate(&model, item->position);
        ska_matrix4_scale(&model, item->scale);
        ska_shader_set_mat4_float(pointLightShader, "model", &model);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
}

void ska_renderer_set_camera_position(SkaVector3 position) {
    currentCamera3D.position = position;
}

SkaCamera3D* ska_renderer_get_active_camera() {
    return &currentCamera3D;
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
    const f32 aspectRatio = (f32)resolution.w / (f32)resolution.h;
    *matrix4 = ska_matrix4_perspective(SKA_DEG_2_RADF(camera->fov), aspectRatio, camera->nearPlane, camera->farPlane);
}

void ska_renderer_flush_meshes() {
    // ska_renderer_get_camera_projection_matrix(ska_renderer_get_active_camera(), &projectionMatrix);
    // ska_renderer_get_camera_view_matrix(ska_renderer_get_active_camera(), &viewMatrix);
    if (meshBatchItemCount == 0) return;

    renderer_batching_draw_meshes(meshBatchItems, meshBatchItemCount);
    meshBatchItemCount = 0;
}

void ska_renderer_flush_lights() {
    renderer_draw_queued_pointLights();
    lightBatchItems.directionalLightCount = 0;
    lightBatchItems.pointLightCount = 0;
    lightBatchItems.spotLightCount = 0;
}

#endif // #if SKA_RENDERING
