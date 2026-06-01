#include "math.h"

#include <math.h>
#include <stdlib.h>

// --- Vector2 --- //
bool ska_math_vec2_equals(const SkaVector2* v1, const SkaVector2* v2) {
    return v1->x == v2->x && v1->y == v2->y;
}

SkaVector2 ska_math_vec2_lerp(const SkaVector2* v1, const SkaVector2* v2, f32 alpha) {
    return (SkaVector2) {
            .x = ska_math_lerpf(v1->x, v2->x, alpha),
            .y = ska_math_lerpf(v1->y, v2->y, alpha)
    };
}

f32 ska_math_vec2_angle(const SkaVector2* v) {
    f32 angle = atan2f(v->y, v->x);
    if (angle < 0.0f) {
        angle += 2.0f * SKA_PI;
    }
    return angle;
}

// --- Vector3 --- //
SkaVector3 ska_vector3_add(SkaVector3 a, SkaVector3 b) {
    return (SkaVector3){ a.x + b.x, a.y + b.y, a.z + b.z };
}

SkaVector3 ska_vector3_sub(SkaVector3 a, SkaVector3 b) {
    return (SkaVector3){ a.x - b.x, a.y - b.y, a.z - b.z };
}

SkaVector3 ska_vector3_mult(SkaVector3 a, SkaVector3 b) {
    return (SkaVector3){ a.x * b.x, a.y * b.y, a.z * b.z };
}

SkaVector3 ska_vector3_cross(SkaVector3 a, SkaVector3 b) {
    return (SkaVector3){
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}

f32 ska_vector3_dot(SkaVector3 a, SkaVector3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

f32 ska_vector3_length(SkaVector3 v) {
    return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}

SkaVector3 ska_vector3_normalize(SkaVector3 v) {
    const f32 len = ska_vector3_length(v);
    if (len == 0.0f) {
        return (SkaVector3){ 0.0f, 0.0f, 0.0f };
    }
    return (SkaVector3){
        v.x / len,
        v.y / len,
        v.z / len
    };
}

// --- Rect2 --- //
bool se_rect2_does_rectangles_overlap(const SkaRect2* sourceRect, const SkaRect2* targetRect) {
    return (sourceRect->x + sourceRect->w >= targetRect->x) &&
           (targetRect->x + targetRect->w >= sourceRect->x) &&
           (sourceRect->y + sourceRect->h >= targetRect->y) &&
           (targetRect->y + targetRect->h >= sourceRect->y);
}

// --- Transform2D --- //

void ska_transform2d_mat4_to_transform(const SkaMatrix4* matrix, SkaTransform2D* transform) {
    // Get the translation from the fourth column
    transform->position.x = matrix->data[3][0];
    transform->position.y = matrix->data[3][1];
    // Get scale: length of the first and seconds columns
    transform->scale.x = sqrtf(matrix->data[0][0] * matrix->data[0][0] + matrix->data[0][1] * matrix->data[0][1]);
    transform->scale.y = sqrtf(matrix->data[1][0] * matrix->data[1][0] + matrix->data[1][1] * matrix->data[1][1]);
    // Get rotation from the first column
    transform->rotation = atan2f(matrix->data[0][1], matrix->data[0][0]);
}

void ska_transform2d_transform_to_mat4(const SkaTransform2D* transform, SkaMatrix4* matrix) {
    // Create translation matrix
    ska_matrix4_translate(matrix, (SkaVector3){transform->position.x, transform->position.y, 0.0f});
    // Create rotation matrix (convert degrees to radians)
    ska_matrix4_rotate_z(matrix, SKA_RAD_2_DEGF(transform->rotation));
    // Create scale matrix
    ska_matrix4_scale(matrix, (SkaVector3){transform->scale.x, transform->scale.y, 1.0f});
}

SkaTransform2D ska_transform2d_lerp(const SkaTransform2D* tA, const SkaTransform2D* tB, f32 alpha) {
    return (SkaTransform2D) {
            .position = ska_math_vec2_lerp(&tA->position, &tB->position, alpha),
            .scale = ska_math_vec2_lerp(&tA->scale, &tB->scale, alpha),
            .rotation = ska_math_lerpf(tA->rotation, tB->rotation, alpha)
    };
}

// --- Transform2D Model --- //
SkaTransform2D ska_transform2d_model_convert_to_transform(const SkaTransformModel2D* transformModel2D) {
    SkaTransform2D transform2D;
    ska_transform2d_mat4_to_transform(&transformModel2D->model, &transform2D);
    // Update scale sign
    transform2D.scale = (SkaVector2){ .x = transform2D.scale.x * transformModel2D->scaleSign.x, .y = transform2D.scale.y * transformModel2D->scaleSign.y };
    return transform2D;
}

// --- Color --- //
SkaColor ska_color_get_normalized_color_default_alpha(uint32 r, uint32 g, uint32 b) {
    SkaColor color = {
            .r = (f32) r / 255.0f,
            .g = (f32) g / 255.0f,
            .b = (f32) b / 255.0f,
            .a = 1.0f
    };
    return color;
}

SkaColor ska_color_get_normalized_color(uint32 r, uint32 g, uint32 b, uint32 a) {
    SkaColor color = {
            .r = (f32) r / 255.0f,
            .g = (f32) g / 255.0f,
            .b = (f32) b / 255.0f,
            .a = (f32) a / 255.0f
    };
    return color;
}

SkaColor ska_color_get_normalized_color_from_color(const SkaColor* color) {
    SkaColor newColor = {
            .r = color->r / 255.0f,
            .g = color->g / 255.0f,
            .b = color->b / 255.0f,
            .a = color->a / 255.0f
    };
    return newColor;
}

SkaVector2 ska_math_minmax_vec2_get_random_in_range(const SkaMinMaxVec2* minmax) {
    const SkaVector2 randomVector = {
            .x = minmax->min.x + (f32)rand() / RAND_MAX * (minmax->max.x - minmax->min.x),
            .y = minmax->min.y + (f32)rand() / RAND_MAX * (minmax->max.y - minmax->min.y),
    };
    return randomVector;
}

// --- SkaMatrix4 --- //

bool ska_matrix4_equals(const SkaMatrix4* a, const SkaMatrix4* b) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (a->data[i][j] != b->data[i][j]) {
                return false;
            }
        }
    }
    return true;
}

SkaMatrix4 ska_matrix4_add(const SkaMatrix4* a, const SkaMatrix4* b) {
    SkaMatrix4 result;
    result.data[0][0] = a->data[0][0] + b->data[0][0];
    result.data[0][1] = a->data[0][1] + b->data[0][1];
    result.data[0][2] = a->data[0][2] + b->data[0][2];
    result.data[0][3] = a->data[0][3] + b->data[0][3];

    result.data[1][0] = a->data[1][0] + b->data[1][0];
    result.data[1][1] = a->data[1][1] + b->data[1][1];
    result.data[1][2] = a->data[1][2] + b->data[1][2];
    result.data[1][3] = a->data[1][3] + b->data[1][3];

    result.data[2][0] = a->data[2][0] + b->data[2][0];
    result.data[2][1] = a->data[2][1] + b->data[2][1];
    result.data[2][2] = a->data[2][2] + b->data[2][2];
    result.data[2][3] = a->data[2][3] + b->data[2][3];

    result.data[3][0] = a->data[3][0] + b->data[3][0];
    result.data[3][1] = a->data[3][1] + b->data[3][1];
    result.data[3][2] = a->data[3][2] + b->data[3][2];
    result.data[3][3] = a->data[3][3] + b->data[3][3];

    return result;
}

SkaMatrix4 ska_matrix4_sub(const SkaMatrix4* a, const SkaMatrix4* b) {
    SkaMatrix4 result;
    result.data[0][0] = a->data[0][0] - b->data[0][0];
    result.data[0][1] = a->data[0][1] - b->data[0][1];
    result.data[0][2] = a->data[0][2] - b->data[0][2];
    result.data[0][3] = a->data[0][3] - b->data[0][3];

    result.data[1][0] = a->data[1][0] - b->data[1][0];
    result.data[1][1] = a->data[1][1] - b->data[1][1];
    result.data[1][2] = a->data[1][2] - b->data[1][2];
    result.data[1][3] = a->data[1][3] - b->data[1][3];

    result.data[2][0] = a->data[2][0] - b->data[2][0];
    result.data[2][1] = a->data[2][1] - b->data[2][1];
    result.data[2][2] = a->data[2][2] - b->data[2][2];
    result.data[2][3] = a->data[2][3] - b->data[2][3];

    result.data[3][0] = a->data[3][0] - b->data[3][0];
    result.data[3][1] = a->data[3][1] - b->data[3][1];
    result.data[3][2] = a->data[3][2] - b->data[3][2];
    result.data[3][3] = a->data[3][3] - b->data[3][3];

    return result;
}

SkaMatrix4 ska_matrix4_mul(const SkaMatrix4* a, const SkaMatrix4* b) {
    SkaMatrix4 result;

    for (int c = 0; c < 4; c++) {
        for (int r = 0; r < 4; r++) {
            f32 sum = 0.0f;
            for (int k = 0; k < 4; k++) {
                // a->data[k][r] is element (r,k) of a
                // b->data[c][k] is element (k,c) of b
                sum += a->data[k][r] * b->data[c][k];
            }
            result.data[c][r] = sum;
        }
    }

    return result;
}

void ska_matrix4_mul_inplace(SkaMatrix4* self, const SkaMatrix4* b) {
    *self = ska_matrix4_mul(self, b);
}

void ska_matrix4_copy_implace(SkaMatrix4* self, const SkaMatrix4* b) {
    *self = *b;
}

void ska_matrix4_translate(SkaMatrix4* self, SkaVector3 v) {
    self->data[3][0] = v.x;
    self->data[3][1] = v.y;
    self->data[3][2] = v.z;
}

void ska_matrix4_translate_and_mul(SkaMatrix4* self, SkaVector3 v) {
    SkaMatrix4 transMat = SKA_MATRIX4_IDENTITY;
    ska_matrix4_translate(&transMat, v);
    ska_matrix4_mul_inplace(self, &transMat);
}

void ska_matrix4_translate_offset(SkaMatrix4* self, SkaVector3 v) {
    self->data[3][0] += v.x;
    self->data[3][1] += v.y;
    self->data[3][2] += v.z;
}

void ska_matrix4_rotate_x(SkaMatrix4* self, f32 angle_rad) {
    const f32 c = cosf(angle_rad);
    const f32 s = sinf(angle_rad);
    self->data[1][1] = c;
    self->data[1][2] = s;
    self->data[2][1] = -s;
    self->data[2][2] = c;
}

void ska_matrix4_rotate_y(SkaMatrix4* self, f32 angle_rad) {
    const f32 c = cosf(angle_rad);
    const f32 s = sinf(angle_rad);
    self->data[0][0] = c;
    self->data[0][2] = -s;
    self->data[2][0] = s;
    self->data[2][2] = c;
}

void ska_matrix4_rotate_y_and_mul(SkaMatrix4* self, f32 angle_rad) {
    SkaMatrix4 rotMat = SKA_MATRIX4_IDENTITY;
    ska_matrix4_rotate_y(&rotMat, angle_rad);
    ska_matrix4_mul_inplace(self, &rotMat);
}

void ska_matrix4_rotate_z(SkaMatrix4* self, f32 angle_rad) {
    const f32 c = cosf(angle_rad);
    const f32 s = sinf(angle_rad);
    self->data[0][0] = c;
    self->data[0][1] = s;
    self->data[1][0] = -s;
    self->data[1][1] = c;
}

void ska_matrix4_rotate_z_and_mul(SkaMatrix4* self, f32 angle_rad) {
    SkaMatrix4 rotMat = SKA_MATRIX4_IDENTITY;
    ska_matrix4_rotate_z(&rotMat, angle_rad);
    ska_matrix4_mul_inplace(self, &rotMat);
}

void ska_matrix4_rotate_yz(SkaMatrix4* self, f32 y_angle_rad, f32 z_angle_rad) {
    // Apply Y rotation first, then Z
    ska_matrix4_rotate_y(self, y_angle_rad);

    SkaMatrix4 rotZ = SKA_MATRIX4_IDENTITY;
    ska_matrix4_rotate_z(&rotZ, z_angle_rad);
    ska_matrix4_mul_inplace(self, &rotZ);
}

void ska_matrix4_rotate_yz_and_mul(SkaMatrix4* self, f32 y_angle_rad, f32 z_angle_rad) {
    SkaMatrix4 rotY = SKA_MATRIX4_IDENTITY;
    ska_matrix4_rotate_y(&rotY, y_angle_rad);

    SkaMatrix4 rotZ = SKA_MATRIX4_IDENTITY;
    ska_matrix4_rotate_z(&rotZ, z_angle_rad);

    SkaMatrix4 combined = ska_matrix4_mul(&rotY, &rotZ);
    ska_matrix4_mul_inplace(self, &combined);
}

void ska_matrix4_rotate_euler(SkaMatrix4* self, f32 x_angle, f32 y_angle, f32 z_angle) {
    // Apply in order: Z -> Y -> X (standard Euler angle order)
    SkaMatrix4 rotZ = SKA_MATRIX4_IDENTITY;
    ska_matrix4_rotate_z(&rotZ, z_angle);

    SkaMatrix4 rotY = SKA_MATRIX4_IDENTITY;
    ska_matrix4_rotate_y(&rotY, y_angle);

    SkaMatrix4 rotX = SKA_MATRIX4_IDENTITY;
    ska_matrix4_rotate_x(&rotX, x_angle);

    // Combine: Z * Y * X
    const SkaMatrix4 temp = ska_matrix4_mul(&rotZ, &rotY);
    const SkaMatrix4 combined = ska_matrix4_mul(&temp, &rotX);

    ska_matrix4_mul_inplace(self, &combined);
}

void ska_matrix4_scale(SkaMatrix4* self, SkaVector3 v) {
    self->data[0][0] = v.x;
    self->data[1][1] = v.y;
    self->data[2][2] = v.z;
}

void ska_matrix4_scale_and_mul(SkaMatrix4* self, SkaVector3 v) {
    SkaMatrix4 scaleMat = SKA_MATRIX4_IDENTITY;
    ska_matrix4_scale(&scaleMat, v);
    ska_matrix4_mul_inplace(self, &scaleMat);
}

static f32 det3(f32 m[3][3]) {
    return m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1])
         - m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0])
         + m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);
}

f32 ska_matrix4_determinant(const SkaMatrix4* self) {
    f32 m0[3][3];
    m0[0][0] = self->data[1][0]; m0[0][1] = self->data[1][1]; m0[0][2] = self->data[1][2];
    m0[1][0] = self->data[2][0]; m0[1][1] = self->data[2][1]; m0[1][2] = self->data[2][2];
    m0[2][0] = self->data[3][0]; m0[2][1] = self->data[3][1]; m0[2][2] = self->data[3][2];
    const f32 det0 = self->data[0][0] * det3(m0);

    f32 m1[3][3];
    m1[0][0] = self->data[1][0]; m1[0][1] = self->data[1][1]; m1[0][2] = self->data[1][3];
    m1[1][0] = self->data[2][0]; m1[1][1] = self->data[2][1]; m1[1][2] = self->data[2][3];
    m1[2][0] = self->data[3][0]; m1[2][1] = self->data[3][1]; m1[2][2] = self->data[3][3];
    const f32 det1 = self->data[0][1] * det3(m1);

    f32 m2[3][3];
    m2[0][0] = self->data[1][0]; m2[0][1] = self->data[1][2]; m2[0][2] = self->data[1][3];
    m2[1][0] = self->data[2][0]; m2[1][1] = self->data[2][2]; m2[1][2] = self->data[2][3];
    m2[2][0] = self->data[3][0]; m2[2][1] = self->data[3][2]; m2[2][2] = self->data[3][3];
    const f32 det2 = self->data[0][2] * det3(m2);

    f32 m3[3][3];
    m3[0][0] = self->data[1][1]; m3[0][1] = self->data[1][2]; m3[0][2] = self->data[1][3];
    m3[1][0] = self->data[2][1]; m3[1][1] = self->data[2][2]; m3[1][2] = self->data[2][3];
    m3[2][0] = self->data[3][1]; m3[2][1] = self->data[3][2]; m3[2][2] = self->data[3][3];
    const f32 det3_val = self->data[0][3] * det3(m3);

    return det0 - det1 + det2 - det3_val;
}

SkaMatrix4 ska_matrix4_inverse(const SkaMatrix4* self) {
    // Flatten to array for easier indexing
    f32 m[16];
    int idx = 0;
    for (int c = 0; c < 4; c++) {
        for (int r = 0; r < 4; r++) {
            m[idx++] = self->data[c][r];
        }
    }

    f32 inv[16];

    inv[0] = m[5]  * m[10] * m[15] -
             m[5]  * m[11] * m[14] -
             m[9]  * m[6]  * m[15] +
             m[9]  * m[7]  * m[14] +
             m[13] * m[6]  * m[11] -
             m[13] * m[7]  * m[10];

    inv[4] = -m[4]  * m[10] * m[15] +
              m[4]  * m[11] * m[14] +
              m[8]  * m[6]  * m[15] -
              m[8]  * m[7]  * m[14] -
              m[12] * m[6]  * m[11] +
              m[12] * m[7]  * m[10];

    inv[8] = m[4]  * m[9]  * m[15] -
             m[4]  * m[11] * m[13] -
             m[8]  * m[5]  * m[15] +
             m[8]  * m[7]  * m[13] +
             m[12] * m[5]  * m[11] -
             m[12] * m[7]  * m[9];

    inv[12] = -m[4]  * m[9]  * m[14] +
               m[4]  * m[10] * m[13] +
               m[8]  * m[5]  * m[14] -
               m[8]  * m[6]  * m[13] -
               m[12] * m[5]  * m[10] +
               m[12] * m[6]  * m[9];

    inv[1] = -m[1]  * m[10] * m[15] +
              m[1]  * m[11] * m[14] +
              m[9]  * m[2]  * m[15] -
              m[9]  * m[3]  * m[14] -
              m[13] * m[2]  * m[11] +
              m[13] * m[3]  * m[10];

    inv[5] = m[0]  * m[10] * m[15] -
             m[0]  * m[11] * m[14] -
             m[8]  * m[2]  * m[15] +
             m[8]  * m[3]  * m[14] +
             m[12] * m[2]  * m[11] -
             m[12] * m[3]  * m[10];

    inv[9] = -m[0]  * m[9]  * m[15] +
              m[0]  * m[11] * m[13] +
              m[8]  * m[1]  * m[15] -
              m[8]  * m[3]  * m[13] -
              m[12] * m[1]  * m[11] +
              m[12] * m[3]  * m[9];

    inv[13] = m[0]  * m[9]  * m[14] -
              m[0]  * m[10] * m[13] -
              m[8]  * m[1]  * m[14] +
              m[8]  * m[2]  * m[13] +
              m[12] * m[1]  * m[10] -
              m[12] * m[2]  * m[9];

    inv[2] = m[1]  * m[6]  * m[15] -
             m[1]  * m[7]  * m[14] -
             m[5]  * m[2]  * m[15] +
             m[5]  * m[3]  * m[14] +
             m[13] * m[2]  * m[7]  -
             m[13] * m[3]  * m[6];

    inv[6] = -m[0]  * m[6]  * m[15] +
              m[0]  * m[7]  * m[14] +
              m[4]  * m[2]  * m[15] -
              m[4]  * m[3]  * m[14] -
              m[12] * m[2]  * m[7]  +
              m[12] * m[3]  * m[6];

    inv[10] = m[0]  * m[5]  * m[15] -
              m[0]  * m[7]  * m[13] -
              m[4]  * m[1]  * m[15] +
              m[4]  * m[3]  * m[13] +
              m[12] * m[1]  * m[7]  -
              m[12] * m[3]  * m[5];

    inv[14] = -m[0]  * m[5]  * m[14] +
               m[0]  * m[6]  * m[13] +
               m[4]  * m[1]  * m[14] -
               m[4]  * m[2]  * m[13] -
               m[12] * m[1]  * m[6]  +
               m[12] * m[2]  * m[5];

    inv[3] = -m[1] * m[6] * m[11] +
              m[1] * m[7] * m[10] +
              m[5] * m[2] * m[11] -
              m[5] * m[3] * m[10] -
              m[9] * m[2] * m[7]  +
              m[9] * m[3] * m[6];

    inv[7] = m[0] * m[6] * m[11] -
             m[0] * m[7] * m[10] -
             m[4] * m[2] * m[11] +
             m[4] * m[3] * m[10] +
             m[8] * m[2] * m[7]  -
             m[8] * m[3] * m[6];

    inv[11] = -m[0] * m[5] * m[11] +
               m[0] * m[7] * m[9]  +
               m[4] * m[1] * m[11] -
               m[4] * m[3] * m[9]  -
               m[8] * m[1] * m[7]  +
               m[8] * m[3] * m[5];

    inv[15] = m[0] * m[5] * m[10] -
              m[0] * m[6] * m[9]  -
              m[4] * m[1] * m[10] +
              m[4] * m[2] * m[9]  +
              m[8] * m[1] * m[6]  -
              m[8] * m[2] * m[5];

    const f32 det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];

    if (det == 0.0f) {
        // Return identity matrix if non-invertible
        return SKA_MATRIX4_IDENTITY;
    }

    const f32 invDet = 1.0f / det;
    SkaMatrix4 result = SKA_MATRIX4_IDENTITY;

    for (int c = 0; c < 4; c++) {
        for (int r = 0; r < 4; r++) {
            result.data[c][r] = inv[c * 4 + r] * invDet;
        }
    }

    return result;
}

SkaMatrix4 ska_matrix4_ortho(f32 left, f32 right, f32 bottom, f32 top, f32 nearZ, f32 farZ) {
    SkaMatrix4 result;

    result.data[0][0] = 2.0f / (right - left);
    result.data[0][1] = 0.0f;
    result.data[0][2] = 0.0f;
    result.data[0][3] = 0.0f;

    result.data[1][0] = 0.0f;
    result.data[1][1] = 2.0f / (top - bottom);
    result.data[1][2] = 0.0f;
    result.data[1][3] = 0.0f;

    result.data[2][0] = 0.0f;
    result.data[2][1] = 0.0f;
    result.data[2][2] = -2.0f / (farZ - nearZ);
    result.data[2][3] = 0.0f;

    result.data[3][0] = -(right + left) / (right - left);
    result.data[3][1] = -(top + bottom) / (top - bottom);
    result.data[3][2] = -(farZ + nearZ) / (farZ - nearZ);
    result.data[3][3] = 1.0f;

    return result;
}

SkaMatrix4 ska_matrix4_perspective(f32 fov_radians, f32 aspect_ratio, f32 near_plane, f32 far_plane) {
    SkaMatrix4 result = SKA_MATRIX4_ZERO;

    const f32 tan_half_fov = tanf(fov_radians / 2.0f);
    result.data[0][0] = 1.0f / (aspect_ratio * tan_half_fov);
    result.data[1][1] = 1.0f / tan_half_fov;
    result.data[2][2] = -(far_plane + near_plane) / (far_plane - near_plane);
    result.data[2][3] = -1.0f;
    result.data[3][2] = -(2.0f * far_plane * near_plane) / (far_plane - near_plane);

    return result;
}

SkaMatrix4 ska_matrix4_look_at(SkaVector3 eye, SkaVector3 center, SkaVector3 up) {
    // Calculate forward vector (from eye to center)
    const SkaVector3 f = ska_vector3_normalize(
        (SkaVector3){ center.x - eye.x, center.y - eye.y, center.z - eye.z }
    );

    // Calculate right vector (cross product of forward and up)
    const SkaVector3 r = ska_vector3_normalize(ska_vector3_cross(f, up));

    // Calculate true up vector (cross product of right and forward)
    const SkaVector3 u = ska_vector3_cross(r, f);

    SkaMatrix4 result = SKA_MATRIX4_IDENTITY;

    result.data[0][0] = r.x;
    result.data[1][0] = r.y;
    result.data[2][0] = r.z;

    result.data[0][1] = u.x;
    result.data[1][1] = u.y;
    result.data[2][1] = u.z;

    result.data[0][2] = -f.x;
    result.data[1][2] = -f.y;
    result.data[2][2] = -f.z;

    result.data[3][0] = -ska_vector3_dot(r, eye);
    result.data[3][1] = -ska_vector3_dot(u, eye);
    result.data[3][2] = ska_vector3_dot(f, eye);

    return result;
}

// --- Misc --- //
f32 ska_math_lerpf(f32 a, f32 b, f32 alpha) {
    return a + (b - a) * alpha;
}

f32 ska_math_map_to_range(f32 input, f32 inputMin, f32 inputMax, f32 outputMin, f32 outputMax) {
    return (((input - inputMin) / (inputMax - inputMin)) * (outputMax - outputMin) + outputMin);
}

f32 ska_math_map_to_unit(f32 input, f32 inputMin, f32 inputMax) {
    return ska_math_map_to_range(input, inputMin, inputMax, 0.0f, 1.0f);
}

f64 ska_math_map_to_range_double(f64 input, f64 inputMin, f64 inputMax, f64 outputMin, f64 outputMax) {
    return (((input - inputMin) / (inputMax - inputMin)) * (outputMax - outputMin) + outputMin);
}

f64 ska_math_map_to_unit_double(f64 input, f64 inputMin, f64 inputMax) {
    return ska_math_map_to_range_double(input, inputMin, inputMax, 0.0, 1.0);
}

f32 ska_math_signf(f32 value) {
    if (value > 0.0f) {
        return 1.0f;
    } else if(value < 0.0f) {
        return -1.0f;
    }
    return 0.0f;
}

SkaVector2 ska_math_signvec2(SkaVector2* value) {
    SkaVector2 sign_vec = {
            .x = ska_math_signf(value->x),
            .y = ska_math_signf(value->y)
    };
    return sign_vec;
}

int ska_math_clamp_int(int value, int min, int max) {
    return value < min ? min : (value > max ? max : value);
}

f32 ska_math_clamp_float(f32 value, f32 min, f32 max) {
    return value < min ? min : (value > max ? max : value);
}

usize ska_math_clamp_size(usize value, usize min, usize max) {
    return value < min ? min : (value > max ? max : value);
}

bool ska_math_is_almost_equal_float(f32 v1, f32 v2, f32 epsilon) {
    return fabsf(v1 - v2) <= epsilon;
}

bool ska_math_is_almost_equal_float_default(f32 v1, f32 v2) {
    static const f64 epsilon = 0.001f;
    return fabsf(v1 - v2) <= epsilon;
}

bool ska_math_is_almost_equal_double(f64 v1, f64 v2, f64 epsilon) {
    return fabs(v1 - v2) <= epsilon;
}

bool ska_math_is_almost_equal_double_default(f64 v1, f64 v2) {
    static const f64 epsilon = 0.001;
    return fabs(v1 - v2) <= epsilon;
}
