#include <unity.h>
#include <seika/simd/simd.h>
#include <string.h>

void setUp(void) {}
void tearDown(void) {}

void seika_simd_i4_splat_storeu_test(void);
void seika_simd_fill_u32_exact_multiple_test(void);
void seika_simd_fill_u32_remainder_test(void);
void seika_simd_fill_u32_small_count_test(void);
void seika_simd_fill_u32_zero_count_test(void);
void seika_simd_f4_add_test(void);
void seika_simd_f4_sub_test(void);
void seika_simd_f4_mul_test(void);
void seika_simd_f4_div_test(void);
void seika_simd_f4_fmadd_test(void);

i32 main(i32 argv, char** args) {
    UNITY_BEGIN();
    RUN_TEST(seika_simd_i4_splat_storeu_test);
    RUN_TEST(seika_simd_fill_u32_exact_multiple_test);
    RUN_TEST(seika_simd_fill_u32_remainder_test);
    RUN_TEST(seika_simd_fill_u32_small_count_test);
    RUN_TEST(seika_simd_fill_u32_zero_count_test);
    RUN_TEST(seika_simd_f4_add_test);
    RUN_TEST(seika_simd_f4_sub_test);
    RUN_TEST(seika_simd_f4_mul_test);
    RUN_TEST(seika_simd_f4_div_test);
    RUN_TEST(seika_simd_f4_fmadd_test);
    return UNITY_END();
}

void seika_simd_i4_splat_storeu_test(void) {
    u32 out[4] = {0};
    ska_simd_i4_storeu(out, ska_simd_i4_splat(0xDEADBEEF));
    for (i32 i = 0; i < 4; i++) {
        TEST_ASSERT_EQUAL_UINT32(0xDEADBEEF, out[i]);
    }
}

void seika_simd_fill_u32_exact_multiple_test(void) {
    u32 buffer[8];
    memset(buffer, 0, sizeof(buffer));
    ska_simd_fill_u32(buffer, 0x12345678, 8);
    for (i32 i = 0; i < 8; i++) {
        TEST_ASSERT_EQUAL_UINT32(0x12345678, buffer[i]);
    }
}

void seika_simd_fill_u32_remainder_test(void) {
    // Count not a multiple of the SIMD lane width -- exercises the scalar tail loop
    u32 buffer[7];
    memset(buffer, 0, sizeof(buffer));
    ska_simd_fill_u32(buffer, 42, 7);
    for (i32 i = 0; i < 7; i++) {
        TEST_ASSERT_EQUAL_UINT32(42, buffer[i]);
    }
}

void seika_simd_fill_u32_small_count_test(void) {
    // Fewer elements than one SIMD-wide store -- should fall through entirely to the scalar tail
    u32 buffer[3] = {0};
    ska_simd_fill_u32(buffer, 7, 3);
    TEST_ASSERT_EQUAL_UINT32(7, buffer[0]);
    TEST_ASSERT_EQUAL_UINT32(7, buffer[1]);
    TEST_ASSERT_EQUAL_UINT32(7, buffer[2]);
}

void seika_simd_fill_u32_zero_count_test(void) {
    u32 buffer[1] = {99};
    ska_simd_fill_u32(buffer, 1, 0);
    TEST_ASSERT_EQUAL_UINT32(99, buffer[0]); // Untouched
}

void seika_simd_f4_add_test(void) {
    f32 a[4] = {1.0f, 2.0f, 3.0f, 4.0f};
    f32 b[4] = {10.0f, 20.0f, 30.0f, 40.0f};
    f32 out[4] = {0};
    ska_simd_f4_storeu(out, ska_simd_f4_add(ska_simd_f4_loadu(a), ska_simd_f4_loadu(b)));
    TEST_ASSERT_EQUAL_FLOAT(11.0f, out[0]);
    TEST_ASSERT_EQUAL_FLOAT(22.0f, out[1]);
    TEST_ASSERT_EQUAL_FLOAT(33.0f, out[2]);
    TEST_ASSERT_EQUAL_FLOAT(44.0f, out[3]);
}

void seika_simd_f4_sub_test(void) {
    f32 a[4] = {10.0f, 20.0f, 30.0f, 40.0f};
    f32 b[4] = {1.0f, 2.0f, 3.0f, 4.0f};
    f32 out[4] = {0};
    ska_simd_f4_storeu(out, ska_simd_f4_sub(ska_simd_f4_loadu(a), ska_simd_f4_loadu(b)));
    TEST_ASSERT_EQUAL_FLOAT(9.0f, out[0]);
    TEST_ASSERT_EQUAL_FLOAT(18.0f, out[1]);
    TEST_ASSERT_EQUAL_FLOAT(27.0f, out[2]);
    TEST_ASSERT_EQUAL_FLOAT(36.0f, out[3]);
}

void seika_simd_f4_mul_test(void) {
    f32 a[4] = {1.0f, 2.0f, 3.0f, 4.0f};
    f32 b[4] = {2.0f, 3.0f, 4.0f, 5.0f};
    f32 out[4] = {0};
    ska_simd_f4_storeu(out, ska_simd_f4_mul(ska_simd_f4_loadu(a), ska_simd_f4_loadu(b)));
    TEST_ASSERT_EQUAL_FLOAT(2.0f, out[0]);
    TEST_ASSERT_EQUAL_FLOAT(6.0f, out[1]);
    TEST_ASSERT_EQUAL_FLOAT(12.0f, out[2]);
    TEST_ASSERT_EQUAL_FLOAT(20.0f, out[3]);
}

void seika_simd_f4_div_test(void) {
    // Values chosen so results don't land on an exact integer, exercising the actual division
    // rather than something a truncating/rounding bug could accidentally still pass.
    f32 a[4] = {1.0f, 5.0f, 9.0f, 10.0f};
    f32 b[4] = {2.0f, 2.0f, 4.0f, 4.0f};
    f32 out[4] = {0};
    ska_simd_f4_storeu(out, ska_simd_f4_div(ska_simd_f4_loadu(a), ska_simd_f4_loadu(b)));
    TEST_ASSERT_EQUAL_FLOAT(0.5f, out[0]);
    TEST_ASSERT_EQUAL_FLOAT(2.5f, out[1]);
    TEST_ASSERT_EQUAL_FLOAT(2.25f, out[2]);
    TEST_ASSERT_EQUAL_FLOAT(2.5f, out[3]);
}

void seika_simd_f4_fmadd_test(void) {
    f32 a[4] = {1.0f, 2.0f, 3.0f, 4.0f};
    f32 b[4] = {2.0f, 2.0f, 2.0f, 2.0f};
    f32 c[4] = {100.0f, 200.0f, 300.0f, 400.0f};
    f32 out[4] = {0};
    ska_simd_f4_storeu(out, ska_simd_f4_fmadd(ska_simd_f4_loadu(a), ska_simd_f4_loadu(b), ska_simd_f4_loadu(c)));
    TEST_ASSERT_EQUAL_FLOAT(102.0f, out[0]);
    TEST_ASSERT_EQUAL_FLOAT(204.0f, out[1]);
    TEST_ASSERT_EQUAL_FLOAT(306.0f, out[2]);
    TEST_ASSERT_EQUAL_FLOAT(408.0f, out[3]);
}
