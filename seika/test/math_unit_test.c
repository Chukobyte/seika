#include <unity.h>
#include <seika/math/math.h>
#include <seika/math/quaternion.h>
#include <seika/math/easing.h>
#include <math.h>

void setUp(void) {}
void tearDown(void) {}

// ===== VECTOR2 TESTS =====

void seika_vector2_add_test(void) {
    const SkaVector2 a = { 1.0f, 2.0f };
    const SkaVector2 b = { 4.0f, 5.0f };
    const SkaVector2 result = ska_vector2_add(a, b);

    TEST_ASSERT_EQUAL_FLOAT(5.0f, result.x);
    TEST_ASSERT_EQUAL_FLOAT(7.0f, result.y);
}

void seika_vector2_sub_test(void) {
    const SkaVector2 a = { 4.0f, 5.0f };
    const SkaVector2 b = { 1.0f, 2.0f };
    const SkaVector2 result = ska_vector2_sub(a, b);

    TEST_ASSERT_EQUAL_FLOAT(3.0f, result.x);
    TEST_ASSERT_EQUAL_FLOAT(3.0f, result.y);
}

void seika_vector2_mult_test(void) {
    const SkaVector2 a = { 2.0f, 3.0f };
    const SkaVector2 b = { 5.0f, 6.0f };
    const SkaVector2 result = ska_vector2_mult(a, b);

    TEST_ASSERT_EQUAL_FLOAT(10.0f, result.x);
    TEST_ASSERT_EQUAL_FLOAT(18.0f, result.y);
}

void seika_vector2_dot_test(void) {
    const SkaVector2 a = { 1.0f, 2.0f };
    const SkaVector2 b = { 4.0f, 5.0f };
    TEST_ASSERT_EQUAL_FLOAT(14.0f, ska_vector2_dot(a, b));
}

void seika_vector2_length_test(void) {
    const SkaVector2 v = { 3.0f, 4.0f };
    TEST_ASSERT_EQUAL_FLOAT(5.0f, ska_vector2_length(v));
}

void seika_vector2_normalize_test(void) {
    const SkaVector2 v = { 3.0f, 4.0f };
    const SkaVector2 normalized = ska_vector2_normalize(v);
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 1.0f, ska_vector2_length(normalized));
}

void seika_vector2_normalize_zero_length_test(void) {
    const SkaVector2 v = { 0.0f, 0.0f };
    const SkaVector2 normalized = ska_vector2_normalize(v);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, normalized.x);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, normalized.y);
}

// ===== VECTOR3 BASELINE TESTS =====

void seika_vector3_add_test(void) {
    const SkaVector3 a = { 1.0f, 2.0f, 3.0f };
    const SkaVector3 b = { 4.0f, 5.0f, 6.0f };
    const SkaVector3 result = ska_vector3_add(a, b);

    TEST_ASSERT_EQUAL_FLOAT(5.0f, result.x);
    TEST_ASSERT_EQUAL_FLOAT(7.0f, result.y);
    TEST_ASSERT_EQUAL_FLOAT(9.0f, result.z);
}

void seika_vector3_sub_test(void) {
    const SkaVector3 a = { 4.0f, 5.0f, 6.0f };
    const SkaVector3 b = { 1.0f, 2.0f, 3.0f };
    const SkaVector3 result = ska_vector3_sub(a, b);

    TEST_ASSERT_EQUAL_FLOAT(3.0f, result.x);
    TEST_ASSERT_EQUAL_FLOAT(3.0f, result.y);
    TEST_ASSERT_EQUAL_FLOAT(3.0f, result.z);
}

void seika_vector3_mult_test(void) {
    const SkaVector3 a = { 2.0f, 3.0f, 4.0f };
    const SkaVector3 b = { 5.0f, 6.0f, 7.0f };
    const SkaVector3 result = ska_vector3_mult(a, b);

    TEST_ASSERT_EQUAL_FLOAT(10.0f, result.x);
    TEST_ASSERT_EQUAL_FLOAT(18.0f, result.y);
    TEST_ASSERT_EQUAL_FLOAT(28.0f, result.z);
}

void seika_vector3_dot_test(void) {
    const SkaVector3 a = { 1.0f, 2.0f, 3.0f };
    const SkaVector3 b = { 4.0f, 5.0f, 6.0f };
    TEST_ASSERT_EQUAL_FLOAT(32.0f, ska_vector3_dot(a, b));
}

void seika_vector3_cross_test(void) {
    const SkaVector3 result = ska_vector3_cross(SKA_VECTOR3_RIGHT, SKA_VECTOR3_UP);
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.0f, result.x);
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.0f, result.y);
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 1.0f, result.z);
}

void seika_vector3_length_test(void) {
    const SkaVector3 v = { 3.0f, 4.0f, 0.0f };
    TEST_ASSERT_EQUAL_FLOAT(5.0f, ska_vector3_length(v));
}

void seika_vector3_normalize_test(void) {
    const SkaVector3 v = { 3.0f, 4.0f, 0.0f };
    const SkaVector3 normalized = ska_vector3_normalize(v);
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 1.0f, ska_vector3_length(normalized));
}

void seika_vector3_normalize_zero_length_test(void) {
    const SkaVector3 v = { 0.0f, 0.0f, 0.0f };
    const SkaVector3 normalized = ska_vector3_normalize(v);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, normalized.x);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, normalized.y);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, normalized.z);
}

// ===== VECTOR4 TESTS =====

void seika_vector4_add_test(void) {
    const SkaVector4 a = { 1.0f, 2.0f, 3.0f, 4.0f };
    const SkaVector4 b = { 5.0f, 6.0f, 7.0f, 8.0f };
    const SkaVector4 result = ska_vector4_add(a, b);

    TEST_ASSERT_EQUAL_FLOAT(6.0f, result.x);
    TEST_ASSERT_EQUAL_FLOAT(8.0f, result.y);
    TEST_ASSERT_EQUAL_FLOAT(10.0f, result.z);
    TEST_ASSERT_EQUAL_FLOAT(12.0f, result.w);
}

void seika_vector4_sub_test(void) {
    const SkaVector4 a = { 5.0f, 6.0f, 7.0f, 8.0f };
    const SkaVector4 b = { 1.0f, 2.0f, 3.0f, 4.0f };
    const SkaVector4 result = ska_vector4_sub(a, b);

    TEST_ASSERT_EQUAL_FLOAT(4.0f, result.x);
    TEST_ASSERT_EQUAL_FLOAT(4.0f, result.y);
    TEST_ASSERT_EQUAL_FLOAT(4.0f, result.z);
    TEST_ASSERT_EQUAL_FLOAT(4.0f, result.w);
}

void seika_vector4_mult_test(void) {
    const SkaVector4 a = { 1.0f, 2.0f, 3.0f, 4.0f };
    const SkaVector4 b = { 2.0f, 2.0f, 2.0f, 2.0f };
    const SkaVector4 result = ska_vector4_mult(a, b);

    TEST_ASSERT_EQUAL_FLOAT(2.0f, result.x);
    TEST_ASSERT_EQUAL_FLOAT(4.0f, result.y);
    TEST_ASSERT_EQUAL_FLOAT(6.0f, result.z);
    TEST_ASSERT_EQUAL_FLOAT(8.0f, result.w);
}

void seika_vector4_dot_test(void) {
    const SkaVector4 a = { 1.0f, 2.0f, 3.0f, 4.0f };
    const SkaVector4 b = { 5.0f, 6.0f, 7.0f, 8.0f };
    TEST_ASSERT_EQUAL_FLOAT(70.0f, ska_vector4_dot(a, b));
}

void seika_vector4_length_test(void) {
    const SkaVector4 v = { 2.0f, 0.0f, 0.0f, 0.0f };
    TEST_ASSERT_EQUAL_FLOAT(2.0f, ska_vector4_length(v));
}

void seika_vector4_normalize_test(void) {
    const SkaVector4 v = { 0.0f, 3.0f, 4.0f, 0.0f };
    const SkaVector4 normalized = ska_vector4_normalize(v);
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 1.0f, ska_vector4_length(normalized));
}

void seika_vector4_normalize_zero_length_test(void) {
    const SkaVector4 v = SKA_VECTOR4_ZERO;
    const SkaVector4 normalized = ska_vector4_normalize(v);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, normalized.x);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, normalized.y);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, normalized.z);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, normalized.w);
}

void seika_vector4_lerp_test(void) {
    const SkaVector4 a = { 0.0f, 0.0f, 0.0f, 0.0f };
    const SkaVector4 b = { 10.0f, 20.0f, -8.0f, 4.0f };
    const SkaVector4 mid = ska_vector4_lerp(a, b, 0.5f);

    TEST_ASSERT_EQUAL_FLOAT(5.0f, mid.x);
    TEST_ASSERT_EQUAL_FLOAT(10.0f, mid.y);
    TEST_ASSERT_EQUAL_FLOAT(-4.0f, mid.z);
    TEST_ASSERT_EQUAL_FLOAT(2.0f, mid.w);

    const SkaVector4 start = ska_vector4_lerp(a, b, 0.0f);
    TEST_ASSERT_EQUAL_FLOAT(a.x, start.x);
    TEST_ASSERT_EQUAL_FLOAT(a.y, start.y);
    TEST_ASSERT_EQUAL_FLOAT(a.z, start.z);
    TEST_ASSERT_EQUAL_FLOAT(a.w, start.w);

    const SkaVector4 end = ska_vector4_lerp(a, b, 1.0f);
    TEST_ASSERT_EQUAL_FLOAT(b.x, end.x);
    TEST_ASSERT_EQUAL_FLOAT(b.y, end.y);
    TEST_ASSERT_EQUAL_FLOAT(b.z, end.z);
    TEST_ASSERT_EQUAL_FLOAT(b.w, end.w);
}

// ===== MATRIX4 TESTS (SIMD add/sub/mul) =====

void seika_matrix4_add_test(void) {
    SkaMatrix4 a = SKA_MATRIX4_ZERO;
    SkaMatrix4 b = SKA_MATRIX4_ZERO;
    for (int c = 0; c < 4; c++) {
        for (int r = 0; r < 4; r++) {
            a.data[c][r] = (f32)(c * 4 + r);
            b.data[c][r] = (f32)(100 + c * 4 + r);
        }
    }
    const SkaMatrix4 result = ska_matrix4_add(&a, &b);
    for (int c = 0; c < 4; c++) {
        for (int r = 0; r < 4; r++) {
            TEST_ASSERT_EQUAL_FLOAT(a.data[c][r] + b.data[c][r], result.data[c][r]);
        }
    }
}

void seika_matrix4_sub_test(void) {
    SkaMatrix4 a = SKA_MATRIX4_ZERO;
    SkaMatrix4 b = SKA_MATRIX4_ZERO;
    for (int c = 0; c < 4; c++) {
        for (int r = 0; r < 4; r++) {
            a.data[c][r] = (f32)(200 + c * 4 + r);
            b.data[c][r] = (f32)(c * 4 + r);
        }
    }
    const SkaMatrix4 result = ska_matrix4_sub(&a, &b);
    for (int c = 0; c < 4; c++) {
        for (int r = 0; r < 4; r++) {
            TEST_ASSERT_EQUAL_FLOAT(a.data[c][r] - b.data[c][r], result.data[c][r]);
        }
    }
}

void seika_matrix4_mul_identity_test(void) {
    const SkaMatrix4 identity = SKA_MATRIX4_IDENTITY;
    SkaMatrix4 m = SKA_MATRIX4_ZERO;
    for (int c = 0; c < 4; c++) {
        for (int r = 0; r < 4; r++) {
            m.data[c][r] = (f32)(c * 4 + r + 1);
        }
    }
    const SkaMatrix4 result_right = ska_matrix4_mul(&m, &identity);
    const SkaMatrix4 result_left = ska_matrix4_mul(&identity, &m);
    for (int c = 0; c < 4; c++) {
        for (int r = 0; r < 4; r++) {
            TEST_ASSERT_EQUAL_FLOAT(m.data[c][r], result_right.data[c][r]);
            TEST_ASSERT_EQUAL_FLOAT(m.data[c][r], result_left.data[c][r]);
        }
    }
}

void seika_matrix4_mul_translation_and_scale_test(void) {
    // Translate by (10, 20, 0) then scale by (2, 3, 1) -- classic order-dependent check:
    // translate * scale should scale the translation too, unlike scale * translate.
    SkaMatrix4 translate = SKA_MATRIX4_IDENTITY;
    ska_matrix4_translate(&translate, (SkaVector3){10.0f, 20.0f, 0.0f});
    SkaMatrix4 scale = SKA_MATRIX4_IDENTITY;
    ska_matrix4_scale(&scale, (SkaVector3){2.0f, 3.0f, 1.0f});

    const SkaMatrix4 result = ska_matrix4_mul(&translate, &scale);

    // Expect: result = translate composed with scale (scale applied first, in column-vector convention)
    TEST_ASSERT_EQUAL_FLOAT(2.0f, result.data[0][0]);
    TEST_ASSERT_EQUAL_FLOAT(3.0f, result.data[1][1]);
    TEST_ASSERT_EQUAL_FLOAT(1.0f, result.data[2][2]);
    TEST_ASSERT_EQUAL_FLOAT(10.0f, result.data[3][0]);
    TEST_ASSERT_EQUAL_FLOAT(20.0f, result.data[3][1]);
    TEST_ASSERT_EQUAL_FLOAT(1.0f, result.data[3][3]);
}

void seika_matrix4_mul_matches_naive_reference_test(void) {
    // Cross-check the SIMD implementation against the original scalar triple-loop formula
    // (result.data[c][r] = sum_k a.data[k][r] * b.data[c][k]) for arbitrary (non-trivial) matrices.
    SkaMatrix4 a, b;
    f32 seed = 1.0f;
    for (int c = 0; c < 4; c++) {
        for (int r = 0; r < 4; r++) {
            a.data[c][r] = seed * 0.37f - 1.5f;
            b.data[c][r] = seed * -0.21f + 2.25f;
            seed += 1.0f;
        }
    }

    const SkaMatrix4 actual = ska_matrix4_mul(&a, &b);

    for (int c = 0; c < 4; c++) {
        for (int r = 0; r < 4; r++) {
            f32 expected = 0.0f;
            for (int k = 0; k < 4; k++) {
                expected += a.data[k][r] * b.data[c][k];
            }
            TEST_ASSERT_FLOAT_WITHIN(0.001f, expected, actual.data[c][r]);
        }
    }
}

void seika_matrix4_determinant_identity_test(void) {
    // Regression test: a prior version of ska_matrix4_determinant paired each Laplace-expansion
    // element with the wrong minor (skipping row 3-r instead of row r), which zeroed out the
    // identity matrix's determinant to 0 instead of 1.
    const SkaMatrix4 identity = SKA_MATRIX4_IDENTITY;
    TEST_ASSERT_EQUAL_FLOAT(1.0f, ska_matrix4_determinant(&identity));
}

void seika_matrix4_determinant_scale_test(void) {
    SkaMatrix4 m = SKA_MATRIX4_IDENTITY;
    ska_matrix4_scale(&m, (SkaVector3){2.0f, 3.0f, 4.0f});
    TEST_ASSERT_EQUAL_FLOAT(24.0f, ska_matrix4_determinant(&m));
}

void seika_matrix4_determinant_singular_test(void) {
    // A matrix with a zeroed-out row/column is singular -- determinant must be exactly 0.
    SkaMatrix4 m = SKA_MATRIX4_IDENTITY;
    m.data[2][2] = 0.0f;
    TEST_ASSERT_EQUAL_FLOAT(0.0f, ska_matrix4_determinant(&m));
}

// ===== QUATERNION TESTS =====

void seika_quat_identity_test(void) {
    const SkaQuat q = ska_quat_identity();
    TEST_ASSERT_EQUAL_FLOAT(0.0f, q.x);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, q.y);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, q.z);
    TEST_ASSERT_EQUAL_FLOAT(1.0f, q.w);
}

void seika_quat_from_axis_angle_matches_matrix_rotate_z_test(void) {
    const f32 angle = SKA_PI / 2.0f; // 90 degrees
    const SkaQuat q = ska_quat_from_axis_angle((SkaVector3) { 0.0f, 0.0f, 1.0f }, angle);

    SkaMatrix4 expected = SKA_MATRIX4_IDENTITY;
    ska_matrix4_rotate_z(&expected, angle);

    const SkaMatrix4 actual = ska_quat_to_matrix4(q);

    for (int c = 0; c < 3; c++) {
        for (int r = 0; r < 3; r++) {
            TEST_ASSERT_FLOAT_WITHIN(0.0001f, expected.data[c][r], actual.data[c][r]);
        }
    }
}

void seika_quat_rotate_vector3_test(void) {
    const SkaQuat q = ska_quat_from_axis_angle((SkaVector3) { 0.0f, 0.0f, 1.0f }, SKA_PI / 2.0f);
    const SkaVector3 rotated = ska_quat_rotate_vector3(q, SKA_VECTOR3_RIGHT);

    SkaMatrix4 rot_mat = SKA_MATRIX4_IDENTITY;
    ska_matrix4_rotate_z(&rot_mat, SKA_PI / 2.0f);
    const SkaVector3 expected = { rot_mat.data[0][0], rot_mat.data[0][1], rot_mat.data[0][2] };

    TEST_ASSERT_FLOAT_WITHIN(0.0001f, expected.x, rotated.x);
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, expected.y, rotated.y);
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, expected.z, rotated.z);
}

void seika_quat_normalize_test(void) {
    const SkaQuat q = { 2.0f, 0.0f, 0.0f, 0.0f };
    const SkaQuat normalized = ska_quat_normalize(q);
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 1.0f, ska_quat_length(normalized));
}

void seika_quat_normalize_zero_length_test(void) {
    const SkaQuat q = { 0.0f, 0.0f, 0.0f, 0.0f };
    const SkaQuat normalized = ska_quat_normalize(q);
    TEST_ASSERT_TRUE(ska_quat_equals(SKA_QUAT_IDENTITY, normalized));
}

void seika_quat_mul_composes_rotations_test(void) {
    const SkaQuat q90 = ska_quat_from_axis_angle((SkaVector3) { 0.0f, 0.0f, 1.0f }, SKA_PI / 2.0f);
    const SkaQuat q180_composed = ska_quat_mul(q90, q90);
    const SkaQuat q180_direct = ska_quat_from_axis_angle((SkaVector3) { 0.0f, 0.0f, 1.0f }, SKA_PI);

    TEST_ASSERT_TRUE(ska_quat_is_almost_equal(q180_direct, q180_composed, 0.0001f));
}

void seika_quat_inverse_test(void) {
    const SkaQuat q = ska_quat_from_axis_angle((SkaVector3) { 1.0f, 0.0f, 0.0f }, SKA_PI / 3.0f);
    const SkaQuat result = ska_quat_mul(q, ska_quat_inverse(q));
    TEST_ASSERT_TRUE(ska_quat_is_almost_equal(SKA_QUAT_IDENTITY, result, 0.0001f));
}

void seika_quat_slerp_edge_cases_test(void) {
    const SkaQuat a = ska_quat_from_axis_angle((SkaVector3) { 0.0f, 0.0f, 1.0f }, 0.0f);
    const SkaQuat b = ska_quat_from_axis_angle((SkaVector3) { 0.0f, 0.0f, 1.0f }, SKA_PI / 2.0f);

    const SkaQuat at_zero = ska_quat_slerp(a, b, 0.0f);
    TEST_ASSERT_TRUE(ska_quat_is_almost_equal(a, at_zero, 0.0001f));

    const SkaQuat at_one = ska_quat_slerp(a, b, 1.0f);
    TEST_ASSERT_TRUE(ska_quat_is_almost_equal(b, at_one, 0.0001f));

    const SkaQuat same_input = ska_quat_slerp(a, a, 0.5f);
    TEST_ASSERT_TRUE(ska_quat_is_almost_equal(a, same_input, 0.0001f));
}

void seika_quat_to_from_matrix4_round_trip_test(void) {
    const SkaQuat original = ska_quat_from_axis_angle((SkaVector3) { 0.0f, 1.0f, 0.0f }, SKA_PI / 4.0f);
    const SkaMatrix4 matrix = ska_quat_to_matrix4(original);
    const SkaQuat recovered = ska_quat_from_matrix4(&matrix);

    TEST_ASSERT_TRUE(ska_quat_is_almost_equal(original, recovered, 0.0001f) ||
                      ska_quat_is_almost_equal(original, ska_quat_scale(recovered, -1.0f), 0.0001f));
}

// ===== EASING TESTS =====

void seika_ease_linear_test(void) {
    TEST_ASSERT_EQUAL_FLOAT(0.0f, ska_ease_linear(0.0f));
    TEST_ASSERT_EQUAL_FLOAT(0.5f, ska_ease_linear(0.5f));
    TEST_ASSERT_EQUAL_FLOAT(1.0f, ska_ease_linear(1.0f));
}

void seika_ease_sine_endpoints_test(void) {
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.0f, ska_ease_sine_in(0.0f));
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 1.0f, ska_ease_sine_in(1.0f));
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.0f, ska_ease_sine_out(0.0f));
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 1.0f, ska_ease_sine_out(1.0f));
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.0f, ska_ease_sine_in_out(0.0f));
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 1.0f, ska_ease_sine_in_out(1.0f));
}

void seika_ease_quad_test(void) {
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.0f, ska_ease_quad_in(0.0f));
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 1.0f, ska_ease_quad_in(1.0f));
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.25f, ska_ease_quad_in(0.5f));
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.0f, ska_ease_quad_out(0.0f));
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 1.0f, ska_ease_quad_out(1.0f));
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.75f, ska_ease_quad_out(0.5f));
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.0f, ska_ease_quad_in_out(0.0f));
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 1.0f, ska_ease_quad_in_out(1.0f));
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.5f, ska_ease_quad_in_out(0.5f));
}

void seika_ease_cubic_test(void) {
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.0f, ska_ease_cubic_in(0.0f));
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 1.0f, ska_ease_cubic_in(1.0f));
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.125f, ska_ease_cubic_in(0.5f));
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.0f, ska_ease_cubic_out(0.0f));
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 1.0f, ska_ease_cubic_out(1.0f));
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.875f, ska_ease_cubic_out(0.5f));
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.0f, ska_ease_cubic_in_out(0.0f));
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 1.0f, ska_ease_cubic_in_out(1.0f));
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.5f, ska_ease_cubic_in_out(0.5f));
}

void seika_ease_quart_quint_endpoints_test(void) {
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.0f, ska_ease_quart_in(0.0f));
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 1.0f, ska_ease_quart_in(1.0f));
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.0f, ska_ease_quart_out(0.0f));
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 1.0f, ska_ease_quart_out(1.0f));
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.0f, ska_ease_quart_in_out(0.0f));
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 1.0f, ska_ease_quart_in_out(1.0f));

    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.0f, ska_ease_quint_in(0.0f));
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 1.0f, ska_ease_quint_in(1.0f));
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.0f, ska_ease_quint_out(0.0f));
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 1.0f, ska_ease_quint_out(1.0f));
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.0f, ska_ease_quint_in_out(0.0f));
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 1.0f, ska_ease_quint_in_out(1.0f));
}

void seika_ease_expo_endpoints_test(void) {
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.0f, ska_ease_expo_in(0.0f));
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 1.0f, ska_ease_expo_in(1.0f));
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.0f, ska_ease_expo_out(0.0f));
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 1.0f, ska_ease_expo_out(1.0f));
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.0f, ska_ease_expo_in_out(0.0f));
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 1.0f, ska_ease_expo_in_out(1.0f));
}

void seika_ease_circ_test(void) {
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.0f, ska_ease_circ_in(0.0f));
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 1.0f, ska_ease_circ_in(1.0f));
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.0f, ska_ease_circ_out(0.0f));
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 1.0f, ska_ease_circ_out(1.0f));
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.0f, ska_ease_circ_in_out(0.0f));
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 1.0f, ska_ease_circ_in_out(1.0f));
}

void seika_ease_back_overshoots_test(void) {
    // Back easing dips below 0 (ease-in start) / pops above 1 (ease-out end) by design.
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.0f, ska_ease_back_in(0.0f));
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 1.0f, ska_ease_back_in(1.0f));
    TEST_ASSERT_TRUE(ska_ease_back_in(0.2f) < 0.0f);
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.0f, ska_ease_back_out(0.0f));
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 1.0f, ska_ease_back_out(1.0f));
    TEST_ASSERT_TRUE(ska_ease_back_out(0.8f) > 1.0f);
}

void seika_ease_elastic_endpoints_test(void) {
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.0f, ska_ease_elastic_in(0.0f));
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 1.0f, ska_ease_elastic_in(1.0f));
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.0f, ska_ease_elastic_out(0.0f));
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 1.0f, ska_ease_elastic_out(1.0f));
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.0f, ska_ease_elastic_in_out(0.0f));
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 1.0f, ska_ease_elastic_in_out(1.0f));
}

void seika_ease_bounce_test(void) {
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.0f, ska_ease_bounce_out(0.0f));
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 1.0f, ska_ease_bounce_out(1.0f));
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.0f, ska_ease_bounce_in(0.0f));
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 1.0f, ska_ease_bounce_in(1.0f));
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.0f, ska_ease_bounce_in_out(0.0f));
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 1.0f, ska_ease_bounce_in_out(1.0f));
    // bounce_in should be a mirror of bounce_out around t=0.5
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, ska_ease_bounce_in(0.3f), 1.0f - ska_ease_bounce_out(0.7f));
}

// Example: easing a position (SkaVector3) from one point to another over normalized time t.
// Pattern: ease the alpha first, then feed the eased alpha into a plain lerp.
void seika_ease_vector3_position_test(void) {
    const SkaVector3 from = { 0.0f, 0.0f, 0.0f };
    const SkaVector3 to = { 10.0f, 0.0f, -4.0f };

    // t=0 and t=1 should land exactly on the endpoints regardless of curve.
    const f32 eased_start = ska_ease_cubic_in_out(0.0f);
    const SkaVector3 pos_start = ska_vector3_lerp(from, to, eased_start);
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, from.x, pos_start.x);
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, from.z, pos_start.z);

    const f32 eased_end = ska_ease_cubic_in_out(1.0f);
    const SkaVector3 pos_end = ska_vector3_lerp(from, to, eased_end);
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, to.x, pos_end.x);
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, to.z, pos_end.z);

    // Halfway through cubic-in-out time is also halfway through the position (symmetric curve).
    const f32 eased_mid = ska_ease_cubic_in_out(0.5f);
    const SkaVector3 pos_mid = ska_vector3_lerp(from, to, eased_mid);
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 5.0f, pos_mid.x);
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, -2.0f, pos_mid.z);

    // Early on, ease-in-out starts slower than linear (t=0.25 covers less than a quarter of the distance).
    const f32 eased_quarter = ska_ease_cubic_in_out(0.25f);
    const SkaVector3 pos_quarter = ska_vector3_lerp(from, to, eased_quarter);
    TEST_ASSERT_TRUE(pos_quarter.x < ska_math_lerpf(from.x, to.x, 0.25f));
}

void seika_ease_eval_dispatch_matches_direct_test(void) {
    TEST_ASSERT_EQUAL_FLOAT(ska_ease_linear(0.3f), ska_ease_eval(SKA_EASE_LINEAR, SKA_EASE_IN, 0.3f));
    TEST_ASSERT_EQUAL_FLOAT(ska_ease_cubic_in(0.4f), ska_ease_eval(SKA_EASE_CUBIC, SKA_EASE_IN, 0.4f));
    TEST_ASSERT_EQUAL_FLOAT(ska_ease_cubic_out(0.4f), ska_ease_eval(SKA_EASE_CUBIC, SKA_EASE_OUT, 0.4f));
    TEST_ASSERT_EQUAL_FLOAT(ska_ease_cubic_in_out(0.6f), ska_ease_eval(SKA_EASE_CUBIC, SKA_EASE_IN_OUT, 0.6f));
    TEST_ASSERT_EQUAL_FLOAT(ska_ease_elastic_out(0.5f), ska_ease_eval(SKA_EASE_ELASTIC, SKA_EASE_OUT, 0.5f));
    TEST_ASSERT_EQUAL_FLOAT(ska_ease_bounce_in_out(0.7f), ska_ease_eval(SKA_EASE_BOUNCE, SKA_EASE_IN_OUT, 0.7f));
}

// ===== TRANSFORM2D REGRESSION TESTS (Fix 1 / Fix 2) =====

void seika_transform2d_compose_rotation_and_scale_test(void) {
    const SkaTransform2D transform = {
        .position = { 10.0f, 20.0f },
        .scale = { 2.0f, 3.0f },
        .rotation = 90.0f // degrees
    };

    SkaMatrix4 matrix = SKA_MATRIX4_IDENTITY;
    ska_transform2d_transform_to_mat4(&transform, &matrix);

    // Expected: rotate_z(90deg) then scale by (2,3) composed via multiplication
    SkaMatrix4 expected = SKA_MATRIX4_IDENTITY;
    ska_matrix4_translate_and_mul(&expected, (SkaVector3) { 10.0f, 20.0f, 0.0f });
    ska_matrix4_rotate_z_and_mul(&expected, SKA_DEG_2_RADF(90.0f));
    ska_matrix4_scale_and_mul(&expected, (SkaVector3) { 2.0f, 3.0f, 1.0f });

    for (int c = 0; c < 4; c++) {
        for (int r = 0; r < 4; r++) {
            TEST_ASSERT_FLOAT_WITHIN(0.0001f, expected.data[c][r], matrix.data[c][r]);
        }
    }

    // Confirm rotation actually survived the scale (the original bug stomped this to the scale values)
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, matrix.data[0][0]);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 2.0f, matrix.data[0][1]);
}

void seika_transform2d_round_trip_test(void) {
    const SkaTransform2D original = {
        .position = { 5.0f, -3.0f },
        .scale = { 2.0f, 3.0f },
        .rotation = 90.0f
    };

    SkaMatrix4 matrix = SKA_MATRIX4_IDENTITY;
    ska_transform2d_transform_to_mat4(&original, &matrix);

    SkaTransform2D recovered;
    ska_transform2d_mat4_to_transform(&matrix, &recovered);

    TEST_ASSERT_TRUE(ska_math_is_almost_equal_float_default(original.position.x, recovered.position.x));
    TEST_ASSERT_TRUE(ska_math_is_almost_equal_float_default(original.position.y, recovered.position.y));
    TEST_ASSERT_TRUE(ska_math_is_almost_equal_float_default(original.scale.x, recovered.scale.x));
    TEST_ASSERT_TRUE(ska_math_is_almost_equal_float_default(original.scale.y, recovered.scale.y));
    TEST_ASSERT_FLOAT_WITHIN(0.01f, original.rotation, recovered.rotation);
}

void seika_transform2d_round_trip_zero_rotation_test(void) {
    const SkaTransform2D original = {
        .position = { 1.0f, 2.0f },
        .scale = { 1.0f, 1.0f },
        .rotation = 0.0f
    };

    SkaMatrix4 matrix = SKA_MATRIX4_IDENTITY;
    ska_transform2d_transform_to_mat4(&original, &matrix);

    SkaTransform2D recovered;
    ska_transform2d_mat4_to_transform(&matrix, &recovered);

    TEST_ASSERT_TRUE(ska_math_is_almost_equal_float_default(original.position.x, recovered.position.x));
    TEST_ASSERT_TRUE(ska_math_is_almost_equal_float_default(original.position.y, recovered.position.y));
    TEST_ASSERT_TRUE(ska_math_is_almost_equal_float_default(original.scale.x, recovered.scale.x));
    TEST_ASSERT_TRUE(ska_math_is_almost_equal_float_default(original.scale.y, recovered.scale.y));
    TEST_ASSERT_FLOAT_WITHIN(0.01f, original.rotation, recovered.rotation);
}

// ===== MISC FIX TESTS =====

void seika_math_clamp_int_test(void) {
    TEST_ASSERT_EQUAL_INT32(5, ska_math_clamp_int(5, 0, 10));
    TEST_ASSERT_EQUAL_INT32(0, ska_math_clamp_int(-5, 0, 10));
    TEST_ASSERT_EQUAL_INT32(10, ska_math_clamp_int(15, 0, 10));
}

// ===== MAIN TEST RUNNER =====

i32 main(i32 argv, char** args) {
    UNITY_BEGIN();

    // Vector2
    RUN_TEST(seika_vector2_add_test);
    RUN_TEST(seika_vector2_sub_test);
    RUN_TEST(seika_vector2_mult_test);
    RUN_TEST(seika_vector2_dot_test);
    RUN_TEST(seika_vector2_length_test);
    RUN_TEST(seika_vector2_normalize_test);
    RUN_TEST(seika_vector2_normalize_zero_length_test);

    // Vector3 baseline
    RUN_TEST(seika_vector3_add_test);
    RUN_TEST(seika_vector3_sub_test);
    RUN_TEST(seika_vector3_mult_test);
    RUN_TEST(seika_vector3_dot_test);
    RUN_TEST(seika_vector3_cross_test);
    RUN_TEST(seika_vector3_length_test);
    RUN_TEST(seika_vector3_normalize_test);
    RUN_TEST(seika_vector3_normalize_zero_length_test);

    // Vector4
    RUN_TEST(seika_vector4_add_test);
    RUN_TEST(seika_vector4_sub_test);
    RUN_TEST(seika_vector4_mult_test);
    RUN_TEST(seika_vector4_dot_test);
    RUN_TEST(seika_vector4_length_test);
    RUN_TEST(seika_vector4_normalize_test);
    RUN_TEST(seika_vector4_normalize_zero_length_test);
    RUN_TEST(seika_vector4_lerp_test);

    // Matrix4 (SIMD add/sub/mul)
    RUN_TEST(seika_matrix4_add_test);
    RUN_TEST(seika_matrix4_sub_test);
    RUN_TEST(seika_matrix4_mul_identity_test);
    RUN_TEST(seika_matrix4_mul_translation_and_scale_test);
    RUN_TEST(seika_matrix4_mul_matches_naive_reference_test);
    RUN_TEST(seika_matrix4_determinant_identity_test);
    RUN_TEST(seika_matrix4_determinant_scale_test);
    RUN_TEST(seika_matrix4_determinant_singular_test);

    // Quaternion
    RUN_TEST(seika_quat_identity_test);
    RUN_TEST(seika_quat_from_axis_angle_matches_matrix_rotate_z_test);
    RUN_TEST(seika_quat_rotate_vector3_test);
    RUN_TEST(seika_quat_normalize_test);
    RUN_TEST(seika_quat_normalize_zero_length_test);
    RUN_TEST(seika_quat_mul_composes_rotations_test);
    RUN_TEST(seika_quat_inverse_test);
    RUN_TEST(seika_quat_slerp_edge_cases_test);
    RUN_TEST(seika_quat_to_from_matrix4_round_trip_test);

    // Easing
    RUN_TEST(seika_ease_linear_test);
    RUN_TEST(seika_ease_sine_endpoints_test);
    RUN_TEST(seika_ease_quad_test);
    RUN_TEST(seika_ease_cubic_test);
    RUN_TEST(seika_ease_quart_quint_endpoints_test);
    RUN_TEST(seika_ease_expo_endpoints_test);
    RUN_TEST(seika_ease_circ_test);
    RUN_TEST(seika_ease_back_overshoots_test);
    RUN_TEST(seika_ease_elastic_endpoints_test);
    RUN_TEST(seika_ease_bounce_test);
    RUN_TEST(seika_ease_vector3_position_test);
    RUN_TEST(seika_ease_eval_dispatch_matches_direct_test);

    // Transform2D regression (Fix 1 / Fix 2)
    RUN_TEST(seika_transform2d_compose_rotation_and_scale_test);
    RUN_TEST(seika_transform2d_round_trip_test);
    RUN_TEST(seika_transform2d_round_trip_zero_rotation_test);

    // Misc fixes
    RUN_TEST(seika_math_clamp_int_test);

    return UNITY_END();
}
