#include "quaternion.h"

#include <math.h>

SkaQuat ska_quat_identity(void) {
    return SKA_QUAT_IDENTITY;
}

SkaQuat ska_quat_from_axis_angle(SkaVector3 axis, f32 angle_rad) {
    const SkaVector3 normalized_axis = ska_vector3_normalize(axis);
    const f32 half_angle = angle_rad * 0.5f;
    const f32 s = sinf(half_angle);
    const f32 c = cosf(half_angle);
    return (SkaQuat){
        normalized_axis.x * s,
        normalized_axis.y * s,
        normalized_axis.z * s,
        c
    };
}

SkaQuat ska_quat_from_euler(f32 x_angle_rad, f32 y_angle_rad, f32 z_angle_rad) {
    const SkaQuat qx = ska_quat_from_axis_angle(SKA_VECTOR3_RIGHT, x_angle_rad);
    const SkaQuat qy = ska_quat_from_axis_angle(SKA_VECTOR3_UP, y_angle_rad);
    const SkaQuat qz = ska_quat_from_axis_angle((SkaVector3){ 0.0f, 0.0f, 1.0f }, z_angle_rad);
    return ska_quat_mul(ska_quat_mul(qz, qy), qx);
}

SkaQuat ska_quat_from_matrix4(const SkaMatrix4* matrix) {
    const f32 m00 = matrix->data[0][0], m01 = matrix->data[1][0], m02 = matrix->data[2][0];
    const f32 m10 = matrix->data[0][1], m11 = matrix->data[1][1], m12 = matrix->data[2][1];
    const f32 m20 = matrix->data[0][2], m21 = matrix->data[1][2], m22 = matrix->data[2][2];

    const f32 trace = m00 + m11 + m22;
    SkaQuat result;
    if (trace > 0.0f) {
        const f32 s = sqrtf(trace + 1.0f) * 2.0f;
        result.w = 0.25f * s;
        result.x = (m21 - m12) / s;
        result.y = (m02 - m20) / s;
        result.z = (m10 - m01) / s;
    } else if (m00 > m11 && m00 > m22) {
        const f32 s = sqrtf(1.0f + m00 - m11 - m22) * 2.0f;
        result.w = (m21 - m12) / s;
        result.x = 0.25f * s;
        result.y = (m01 + m10) / s;
        result.z = (m02 + m20) / s;
    } else if (m11 > m22) {
        const f32 s = sqrtf(1.0f + m11 - m00 - m22) * 2.0f;
        result.w = (m02 - m20) / s;
        result.x = (m01 + m10) / s;
        result.y = 0.25f * s;
        result.z = (m12 + m21) / s;
    } else {
        const f32 s = sqrtf(1.0f + m22 - m00 - m11) * 2.0f;
        result.w = (m10 - m01) / s;
        result.x = (m02 + m20) / s;
        result.y = (m12 + m21) / s;
        result.z = 0.25f * s;
    }
    return result;
}

SkaQuat ska_quat_add(SkaQuat a, SkaQuat b) {
    return (SkaQuat){ a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w };
}

SkaQuat ska_quat_sub(SkaQuat a, SkaQuat b) {
    return (SkaQuat){ a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w };
}

SkaQuat ska_quat_scale(SkaQuat q, f32 scalar) {
    return (SkaQuat){ q.x * scalar, q.y * scalar, q.z * scalar, q.w * scalar };
}

SkaQuat ska_quat_mul(SkaQuat a, SkaQuat b) {
    return (SkaQuat){
        .x = a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y,
        .y = a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x,
        .z = a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w,
        .w = a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z
    };
}

f32 ska_quat_dot(SkaQuat a, SkaQuat b) {
    return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

f32 ska_quat_length(SkaQuat q) {
    return sqrtf(ska_quat_dot(q, q));
}

SkaQuat ska_quat_normalize(SkaQuat q) {
    const f32 len = ska_quat_length(q);
    if (len == 0.0f) {
        return SKA_QUAT_IDENTITY;
    }
    return (SkaQuat){ q.x / len, q.y / len, q.z / len, q.w / len };
}

SkaQuat ska_quat_conjugate(SkaQuat q) {
    return (SkaQuat){ -q.x, -q.y, -q.z, q.w };
}

SkaQuat ska_quat_inverse(SkaQuat q) {
    return ska_quat_conjugate(ska_quat_normalize(q));
}

SkaQuat ska_quat_lerp(SkaQuat a, SkaQuat b, f32 alpha) {
    SkaQuat target = b;
    if (ska_quat_dot(a, b) < 0.0f) {
        target = (SkaQuat){ -b.x, -b.y, -b.z, -b.w };
    }
    return ska_quat_normalize((SkaQuat){
        ska_math_lerpf(a.x, target.x, alpha),
        ska_math_lerpf(a.y, target.y, alpha),
        ska_math_lerpf(a.z, target.z, alpha),
        ska_math_lerpf(a.w, target.w, alpha)
    });
}

SkaQuat ska_quat_slerp(SkaQuat a, SkaQuat b, f32 alpha) {
    SkaQuat target = b;
    f32 dot = ska_quat_dot(a, b);
    if (dot < 0.0f) {
        target = (SkaQuat){ -b.x, -b.y, -b.z, -b.w };
        dot = -dot;
    }

    // Inputs are nearly parallel: fall back to lerp to avoid dividing by ~0 in sinf(theta)
    if (dot > 0.9995f) {
        return ska_quat_lerp(a, target, alpha);
    }

    const f32 theta0 = acosf(SKA_MATH_CLAMP(dot, -1.0f, 1.0f));
    const f32 theta = theta0 * alpha;
    const f32 sin_theta0 = sinf(theta0);
    const f32 s0 = cosf(theta) - dot * sinf(theta) / sin_theta0;
    const f32 s1 = sinf(theta) / sin_theta0;

    return ska_quat_normalize((SkaQuat){
        a.x * s0 + target.x * s1,
        a.y * s0 + target.y * s1,
        a.z * s0 + target.z * s1,
        a.w * s0 + target.w * s1
    });
}

SkaMatrix4 ska_quat_to_matrix4(SkaQuat q) {
    const SkaQuat nq = ska_quat_normalize(q);
    const f32 xx = nq.x * nq.x, yy = nq.y * nq.y, zz = nq.z * nq.z;
    const f32 xy = nq.x * nq.y, xz = nq.x * nq.z, yz = nq.y * nq.z;
    const f32 wx = nq.w * nq.x, wy = nq.w * nq.y, wz = nq.w * nq.z;

    SkaMatrix4 result = SKA_MATRIX4_IDENTITY;
    result.data[0][0] = 1.0f - 2.0f * (yy + zz);
    result.data[0][1] = 2.0f * (xy + wz);
    result.data[0][2] = 2.0f * (xz - wy);

    result.data[1][0] = 2.0f * (xy - wz);
    result.data[1][1] = 1.0f - 2.0f * (xx + zz);
    result.data[1][2] = 2.0f * (yz + wx);

    result.data[2][0] = 2.0f * (xz + wy);
    result.data[2][1] = 2.0f * (yz - wx);
    result.data[2][2] = 1.0f - 2.0f * (xx + yy);

    return result;
}

SkaVector3 ska_quat_to_euler(SkaQuat q) {
    const SkaQuat nq = ska_quat_normalize(q);
    const f32 sinr_cosp = 2.0f * (nq.w * nq.x + nq.y * nq.z);
    const f32 cosr_cosp = 1.0f - 2.0f * (nq.x * nq.x + nq.y * nq.y);
    const f32 roll = atan2f(sinr_cosp, cosr_cosp);

    const f32 sinp = SKA_MATH_CLAMP(2.0f * (nq.w * nq.y - nq.z * nq.x), -1.0f, 1.0f);
    const f32 pitch = asinf(sinp);

    const f32 siny_cosp = 2.0f * (nq.w * nq.z + nq.x * nq.y);
    const f32 cosy_cosp = 1.0f - 2.0f * (nq.y * nq.y + nq.z * nq.z);
    const f32 yaw = atan2f(siny_cosp, cosy_cosp);

    return (SkaVector3){ roll, pitch, yaw };
}

SkaVector3 ska_quat_rotate_vector3(SkaQuat q, SkaVector3 v) {
    const SkaVector3 qv = { q.x, q.y, q.z };
    SkaVector3 t = ska_vector3_cross(qv, v);
    t = (SkaVector3){ t.x * 2.0f, t.y * 2.0f, t.z * 2.0f };
    const SkaVector3 cross_qv_t = ska_vector3_cross(qv, t);
    return (SkaVector3){
        v.x + q.w * t.x + cross_qv_t.x,
        v.y + q.w * t.y + cross_qv_t.y,
        v.z + q.w * t.z + cross_qv_t.z
    };
}

bool ska_quat_equals(SkaQuat a, SkaQuat b) {
    return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w;
}

bool ska_quat_is_almost_equal(SkaQuat a, SkaQuat b, f32 epsilon) {
    return ska_math_is_almost_equal_float(a.x, b.x, epsilon) &&
           ska_math_is_almost_equal_float(a.y, b.y, epsilon) &&
           ska_math_is_almost_equal_float(a.z, b.z, epsilon) &&
           ska_math_is_almost_equal_float(a.w, b.w, epsilon);
}
