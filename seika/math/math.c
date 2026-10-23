#include "math.h"

#include <math.h>
#include <stdlib.h>

#include "seika/simd/simd.h"

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

SkaVector2 ska_vector2_add(SkaVector2 a, SkaVector2 b) {
    return (SkaVector2){ a.x + b.x, a.y + b.y };
}

SkaVector2 ska_vector2_sub(SkaVector2 a, SkaVector2 b) {
    return (SkaVector2){ a.x - b.x, a.y - b.y };
}

SkaVector2 ska_vector2_mult(SkaVector2 a, SkaVector2 b) {
    return (SkaVector2){ a.x * b.x, a.y * b.y };
}

f32 ska_vector2_dot(SkaVector2 a, SkaVector2 b) {
    return a.x * b.x + a.y * b.y;
}

f32 ska_vector2_length(SkaVector2 v) {
    return sqrtf(v.x * v.x + v.y * v.y);
}

SkaVector2 ska_vector2_normalize(SkaVector2 v) {
    const f32 len = ska_vector2_length(v);
    if (len == 0.0f) {
        return (SkaVector2){ 0.0f, 0.0f };
    }
    return (SkaVector2){ v.x / len, v.y / len };
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

SkaVector3 ska_vector3_lerp(SkaVector3 a, SkaVector3 b, f32 alpha) {
    return (SkaVector3){
        ska_math_lerpf(a.x, b.x, alpha),
        ska_math_lerpf(a.y, b.y, alpha),
        ska_math_lerpf(a.z, b.z, alpha)
    };
}

// --- Vector4 --- //
// SkaVector4 is 4 contiguous f32s, mapping 1:1 onto one SkaSimdF4 lane -- add/sub/mult/normalize/lerp
// route through SIMD. dot/length stay scalar: they're reduction-shaped (need a horizontal sum across
// lanes) and SkaSimdF4 has no such primitive, so composing one here would cost a storeu round-trip for
// no benefit over the current 4-multiply-3-add scalar code.
SkaVector4 ska_vector4_add(SkaVector4 a, SkaVector4 b) {
    SkaVector4 result;
    ska_simd_f4_storeu((f32*)&result, ska_simd_f4_add(ska_simd_f4_loadu((const f32*)&a), ska_simd_f4_loadu((const f32*)&b)));
    return result;
}

SkaVector4 ska_vector4_sub(SkaVector4 a, SkaVector4 b) {
    SkaVector4 result;
    ska_simd_f4_storeu((f32*)&result, ska_simd_f4_sub(ska_simd_f4_loadu((const f32*)&a), ska_simd_f4_loadu((const f32*)&b)));
    return result;
}

SkaVector4 ska_vector4_mult(SkaVector4 a, SkaVector4 b) {
    SkaVector4 result;
    ska_simd_f4_storeu((f32*)&result, ska_simd_f4_mul(ska_simd_f4_loadu((const f32*)&a), ska_simd_f4_loadu((const f32*)&b)));
    return result;
}

f32 ska_vector4_dot(SkaVector4 a, SkaVector4 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

f32 ska_vector4_length(SkaVector4 v) {
    return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w);
}

SkaVector4 ska_vector4_normalize(SkaVector4 v) {
    const f32 len = ska_vector4_length(v);
    if (len == 0.0f) {
        return (SkaVector4){ 0.0f, 0.0f, 0.0f, 0.0f };
    }
    SkaVector4 result;
    ska_simd_f4_storeu((f32*)&result, ska_simd_f4_div(ska_simd_f4_loadu((const f32*)&v), ska_simd_f4_splat(len)));
    return result;
}

SkaVector4 ska_vector4_lerp(SkaVector4 a, SkaVector4 b, f32 alpha) {
    // a + (b - a) * alpha, all lanes at once
    SkaVector4 result;
    const SkaSimdF4 va = ska_simd_f4_loadu((const f32*)&a);
    const SkaSimdF4 diff = ska_simd_f4_sub(ska_simd_f4_loadu((const f32*)&b), va);
    ska_simd_f4_storeu((f32*)&result, ska_simd_f4_fmadd(diff, ska_simd_f4_splat(alpha), va));
    return result;
}

// --- Rect2 --- //
bool se_rect2_does_rectangles_overlap(const SkaRect2* source_rect, const SkaRect2* target_rect) {
    return (source_rect->x + source_rect->w >= target_rect->x) &&
           (target_rect->x + target_rect->w >= source_rect->x) &&
           (source_rect->y + source_rect->h >= target_rect->y) &&
           (target_rect->y + target_rect->h >= source_rect->y);
}

// --- Transform2D --- //

void ska_transform2d_mat4_to_transform(const SkaMatrix4* matrix, SkaTransform2D* transform) {
    // Get the translation from the fourth column
    transform->position.x = matrix->data[3][0];
    transform->position.y = matrix->data[3][1];
    // Get scale: length of the first and seconds columns
    transform->scale.x = sqrtf(matrix->data[0][0] * matrix->data[0][0] + matrix->data[0][1] * matrix->data[0][1]);
    transform->scale.y = sqrtf(matrix->data[1][0] * matrix->data[1][0] + matrix->data[1][1] * matrix->data[1][1]);
    // Get rotation from the first column (convert radians to degrees, rotation field is in degrees)
    transform->rotation = SKA_RAD_2_DEGF(atan2f(matrix->data[0][1], matrix->data[0][0]));
}

void ska_transform2d_transform_to_mat4(const SkaTransform2D* transform, SkaMatrix4* matrix) {
    *matrix = SKA_MATRIX4_IDENTITY;
    // Compose translation, rotation (degrees to radians), and scale via multiplication so each step doesn't overwrite the last
    ska_matrix4_translate_and_mul(matrix, (SkaVector3){transform->position.x, transform->position.y, 0.0f});
    ska_matrix4_rotate_z_and_mul(matrix, SKA_DEG_2_RADF(transform->rotation));
    ska_matrix4_scale_and_mul(matrix, (SkaVector3){transform->scale.x, transform->scale.y, 1.0f});
}

SkaTransform2D ska_transform2d_lerp(const SkaTransform2D* t_a, const SkaTransform2D* t_b, f32 alpha) {
    return (SkaTransform2D) {
            .position = ska_math_vec2_lerp(&t_a->position, &t_b->position, alpha),
            .scale = ska_math_vec2_lerp(&t_a->scale, &t_b->scale, alpha),
            .rotation = ska_math_lerpf(t_a->rotation, t_b->rotation, alpha)
    };
}

// --- Transform2D Model --- //
SkaTransform2D ska_transform2d_model_convert_to_transform(const SkaTransformModel2D* transform_model2_d) {
    SkaTransform2D transform2_d;
    ska_transform2d_mat4_to_transform(&transform_model2_d->model, &transform2_d);
    // Update scale sign
    transform2_d.scale = (SkaVector2){ .x = transform2_d.scale.x * transform_model2_d->scale_sign.x, .y = transform2_d.scale.y * transform_model2_d->scale_sign.y };
    return transform2_d;
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
    SkaColor new_color = {
            .r = color->r / 255.0f,
            .g = color->g / 255.0f,
            .b = color->b / 255.0f,
            .a = color->a / 255.0f
    };
    return new_color;
}

SkaVector2 ska_math_minmax_vec2_get_random_in_range(const SkaMinMaxVec2* minmax) {
    const SkaVector2 random_vector = {
            .x = minmax->min.x + (f32)rand() / RAND_MAX * (minmax->max.x - minmax->min.x),
            .y = minmax->min.y + (f32)rand() / RAND_MAX * (minmax->max.y - minmax->min.y),
    };
    return random_vector;
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
    for (int c = 0; c < 4; c++) {
        ska_simd_f4_storeu(result.data[c], ska_simd_f4_add(ska_simd_f4_loadu(a->data[c]), ska_simd_f4_loadu(b->data[c])));
    }
    return result;
}

SkaMatrix4 ska_matrix4_sub(const SkaMatrix4* a, const SkaMatrix4* b) {
    SkaMatrix4 result;
    for (int c = 0; c < 4; c++) {
        ska_simd_f4_storeu(result.data[c], ska_simd_f4_sub(ska_simd_f4_loadu(a->data[c]), ska_simd_f4_loadu(b->data[c])));
    }
    return result;
}

SkaMatrix4 ska_matrix4_mul(const SkaMatrix4* a, const SkaMatrix4* b) {
    SkaMatrix4 result;

    // Standard SIMD 4x4 multiply: each result column is a linear combination of A's (contiguous)
    // columns, weighted by the scalars in the matching column of B. This computes the exact same
    // sum as the naive triple loop (result.data[c][r] = sum_k a->data[k][r] * b->data[c][k]), just
    // reordered so every lane operation works on 4 contiguous floats instead of a strided gather.
    const SkaSimdF4 a_col0 = ska_simd_f4_loadu(a->data[0]);
    const SkaSimdF4 a_col1 = ska_simd_f4_loadu(a->data[1]);
    const SkaSimdF4 a_col2 = ska_simd_f4_loadu(a->data[2]);
    const SkaSimdF4 a_col3 = ska_simd_f4_loadu(a->data[3]);

    for (int c = 0; c < 4; c++) {
        SkaSimdF4 result_col = ska_simd_f4_mul(a_col0, ska_simd_f4_splat(b->data[c][0]));
        result_col = ska_simd_f4_fmadd(a_col1, ska_simd_f4_splat(b->data[c][1]), result_col);
        result_col = ska_simd_f4_fmadd(a_col2, ska_simd_f4_splat(b->data[c][2]), result_col);
        result_col = ska_simd_f4_fmadd(a_col3, ska_simd_f4_splat(b->data[c][3]), result_col);
        ska_simd_f4_storeu(result.data[c], result_col);
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
    SkaMatrix4 trans_mat = SKA_MATRIX4_IDENTITY;
    ska_matrix4_translate(&trans_mat, v);
    ska_matrix4_mul_inplace(self, &trans_mat);
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
    SkaMatrix4 rot_mat = SKA_MATRIX4_IDENTITY;
    ska_matrix4_rotate_y(&rot_mat, angle_rad);
    ska_matrix4_mul_inplace(self, &rot_mat);
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
    SkaMatrix4 rot_mat = SKA_MATRIX4_IDENTITY;
    ska_matrix4_rotate_z(&rot_mat, angle_rad);
    ska_matrix4_mul_inplace(self, &rot_mat);
}

void ska_matrix4_rotate_yz(SkaMatrix4* self, f32 y_angle_rad, f32 z_angle_rad) {
    // Apply Y rotation first, then Z
    ska_matrix4_rotate_y(self, y_angle_rad);

    SkaMatrix4 rot_z = SKA_MATRIX4_IDENTITY;
    ska_matrix4_rotate_z(&rot_z, z_angle_rad);
    ska_matrix4_mul_inplace(self, &rot_z);
}

void ska_matrix4_rotate_yz_and_mul(SkaMatrix4* self, f32 y_angle_rad, f32 z_angle_rad) {
    SkaMatrix4 rot_y = SKA_MATRIX4_IDENTITY;
    ska_matrix4_rotate_y(&rot_y, y_angle_rad);

    SkaMatrix4 rot_z = SKA_MATRIX4_IDENTITY;
    ska_matrix4_rotate_z(&rot_z, z_angle_rad);

    SkaMatrix4 combined = ska_matrix4_mul(&rot_y, &rot_z);
    ska_matrix4_mul_inplace(self, &combined);
}

void ska_matrix4_rotate_euler(SkaMatrix4* self, f32 x_angle, f32 y_angle, f32 z_angle) {
    // Apply in order: Z -> Y -> X (standard Euler angle order)
    SkaMatrix4 rot_z = SKA_MATRIX4_IDENTITY;
    ska_matrix4_rotate_z(&rot_z, z_angle);

    SkaMatrix4 rot_y = SKA_MATRIX4_IDENTITY;
    ska_matrix4_rotate_y(&rot_y, y_angle);

    SkaMatrix4 rot_x = SKA_MATRIX4_IDENTITY;
    ska_matrix4_rotate_x(&rot_x, x_angle);

    // Combine: Z * Y * X
    const SkaMatrix4 temp = ska_matrix4_mul(&rot_z, &rot_y);
    const SkaMatrix4 combined = ska_matrix4_mul(&temp, &rot_x);

    ska_matrix4_mul_inplace(self, &combined);
}

void ska_matrix4_scale(SkaMatrix4* self, SkaVector3 v) {
    self->data[0][0] = v.x;
    self->data[1][1] = v.y;
    self->data[2][2] = v.z;
}

void ska_matrix4_scale_and_mul(SkaMatrix4* self, SkaVector3 v) {
    SkaMatrix4 scale_mat = SKA_MATRIX4_IDENTITY;
    ska_matrix4_scale(&scale_mat, v);
    ska_matrix4_mul_inplace(self, &scale_mat);
}

static f32 det3(f32 m[3][3]) {
    return m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1])
         - m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0])
         + m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);
}

// Laplace expansion along column 0: det = sum_r (-1)^r * data[0][r] * M_r, where M_r is the
// 3x3 minor formed by columns {1,2,3} with row r removed. Each mN block below must therefore
// skip row N (not row 3-N) -- a prior version of this function had that reversed (mN skipped
// row 3-N instead of row N), which zeroed out every minor whose skipped row happened to be the
// matrix's only non-trivial row (e.g. the identity matrix's determinant silently came out 0
// instead of 1, since three of the four data[0][r] factors are 0 for identity and the one
// surviving term, r=0, multiplied a minor that (incorrectly) still contained an all-zero row).
f32 ska_matrix4_determinant(const SkaMatrix4* self) {
    f32 m0[3][3]; // skips row 0
    m0[0][0] = self->data[1][1]; m0[0][1] = self->data[1][2]; m0[0][2] = self->data[1][3];
    m0[1][0] = self->data[2][1]; m0[1][1] = self->data[2][2]; m0[1][2] = self->data[2][3];
    m0[2][0] = self->data[3][1]; m0[2][1] = self->data[3][2]; m0[2][2] = self->data[3][3];
    const f32 det0 = self->data[0][0] * det3(m0);

    f32 m1[3][3]; // skips row 1
    m1[0][0] = self->data[1][0]; m1[0][1] = self->data[1][2]; m1[0][2] = self->data[1][3];
    m1[1][0] = self->data[2][0]; m1[1][1] = self->data[2][2]; m1[1][2] = self->data[2][3];
    m1[2][0] = self->data[3][0]; m1[2][1] = self->data[3][2]; m1[2][2] = self->data[3][3];
    const f32 det1 = self->data[0][1] * det3(m1);

    f32 m2[3][3]; // skips row 2
    m2[0][0] = self->data[1][0]; m2[0][1] = self->data[1][1]; m2[0][2] = self->data[1][3];
    m2[1][0] = self->data[2][0]; m2[1][1] = self->data[2][1]; m2[1][2] = self->data[2][3];
    m2[2][0] = self->data[3][0]; m2[2][1] = self->data[3][1]; m2[2][2] = self->data[3][3];
    const f32 det2 = self->data[0][2] * det3(m2);

    f32 m3[3][3]; // skips row 3
    m3[0][0] = self->data[1][0]; m3[0][1] = self->data[1][1]; m3[0][2] = self->data[1][2];
    m3[1][0] = self->data[2][0]; m3[1][1] = self->data[2][1]; m3[1][2] = self->data[2][2];
    m3[2][0] = self->data[3][0]; m3[2][1] = self->data[3][1]; m3[2][2] = self->data[3][2];
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

    const f32 inv_det = 1.0f / det;
    SkaMatrix4 result = SKA_MATRIX4_IDENTITY;

    for (int c = 0; c < 4; c++) {
        for (int r = 0; r < 4; r++) {
            result.data[c][r] = inv[c * 4 + r] * inv_det;
        }
    }

    return result;
}

SkaMatrix4 ska_matrix4_ortho(f32 left, f32 right, f32 bottom, f32 top, f32 near_z, f32 far_z) {
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
    result.data[2][2] = -2.0f / (far_z - near_z);
    result.data[2][3] = 0.0f;

    result.data[3][0] = -(right + left) / (right - left);
    result.data[3][1] = -(top + bottom) / (top - bottom);
    result.data[3][2] = -(far_z + near_z) / (far_z - near_z);
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

f32 ska_math_map_to_range(f32 input, f32 input_min, f32 input_max, f32 output_min, f32 output_max) {
    return (((input - input_min) / (input_max - input_min)) * (output_max - output_min) + output_min);
}

f32 ska_math_map_to_unit(f32 input, f32 input_min, f32 input_max) {
    return ska_math_map_to_range(input, input_min, input_max, 0.0f, 1.0f);
}

f64 ska_math_map_to_range_double(f64 input, f64 input_min, f64 input_max, f64 output_min, f64 output_max) {
    return (((input - input_min) / (input_max - input_min)) * (output_max - output_min) + output_min);
}

f64 ska_math_map_to_unit_double(f64 input, f64 input_min, f64 input_max) {
    return ska_math_map_to_range_double(input, input_min, input_max, 0.0, 1.0);
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

int32 ska_math_clamp_int(int32 value, int32 min, int32 max) {
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
