#include "seika/simd/simd.h"

SkaSimdI4 ska_simd_i4_splat(u32 value) {
#if defined(SKA_SIMD_SSE2)
    return _mm_set1_epi32((int)value);
#elif defined(SKA_SIMD_NEON)
    return vdupq_n_u32(value);
#elif defined(SKA_SIMD_WASM)
    return wasm_i32x4_splat((int32_t)value);
#else
    return (SkaSimdI4){.v = {value, value, value, value}};
#endif
}

void ska_simd_i4_storeu(u32* dst, SkaSimdI4 vec) {
#if defined(SKA_SIMD_SSE2)
    _mm_storeu_si128((__m128i*)dst, vec);
#elif defined(SKA_SIMD_NEON)
    vst1q_u32(dst, vec);
#elif defined(SKA_SIMD_WASM)
    wasm_v128_store(dst, vec);
#else
    dst[0] = vec.v[0];
    dst[1] = vec.v[1];
    dst[2] = vec.v[2];
    dst[3] = vec.v[3];
#endif
}

void ska_simd_fill_u32(u32* dst, u32 pattern, usize count) {
    const SkaSimdI4 wide = ska_simd_i4_splat(pattern);
    usize i = 0;
    for (; i + SKA_SIMD_I4_LANES <= count; i += SKA_SIMD_I4_LANES) {
        ska_simd_i4_storeu(dst + i, wide);
    }
    for (; i < count; i++) {
        dst[i] = pattern;
    }
}

SkaSimdF4 ska_simd_f4_splat(f32 value) {
#if defined(SKA_SIMD_SSE2)
    return _mm_set1_ps(value);
#elif defined(SKA_SIMD_NEON)
    return vdupq_n_f32(value);
#elif defined(SKA_SIMD_WASM)
    return wasm_f32x4_splat(value);
#else
    return (SkaSimdF4){.v = {value, value, value, value}};
#endif
}

SkaSimdF4 ska_simd_f4_loadu(const f32* src) {
#if defined(SKA_SIMD_SSE2)
    return _mm_loadu_ps(src);
#elif defined(SKA_SIMD_NEON)
    return vld1q_f32(src);
#elif defined(SKA_SIMD_WASM)
    return wasm_v128_load(src);
#else
    return (SkaSimdF4){.v = {src[0], src[1], src[2], src[3]}};
#endif
}

void ska_simd_f4_storeu(f32* dst, SkaSimdF4 vec) {
#if defined(SKA_SIMD_SSE2)
    _mm_storeu_ps(dst, vec);
#elif defined(SKA_SIMD_NEON)
    vst1q_f32(dst, vec);
#elif defined(SKA_SIMD_WASM)
    wasm_v128_store(dst, vec);
#else
    dst[0] = vec.v[0];
    dst[1] = vec.v[1];
    dst[2] = vec.v[2];
    dst[3] = vec.v[3];
#endif
}

SkaSimdF4 ska_simd_f4_add(SkaSimdF4 a, SkaSimdF4 b) {
#if defined(SKA_SIMD_SSE2)
    return _mm_add_ps(a, b);
#elif defined(SKA_SIMD_NEON)
    return vaddq_f32(a, b);
#elif defined(SKA_SIMD_WASM)
    return wasm_f32x4_add(a, b);
#else
    return (SkaSimdF4){.v = {a.v[0] + b.v[0], a.v[1] + b.v[1], a.v[2] + b.v[2], a.v[3] + b.v[3]}};
#endif
}

SkaSimdF4 ska_simd_f4_sub(SkaSimdF4 a, SkaSimdF4 b) {
#if defined(SKA_SIMD_SSE2)
    return _mm_sub_ps(a, b);
#elif defined(SKA_SIMD_NEON)
    return vsubq_f32(a, b);
#elif defined(SKA_SIMD_WASM)
    return wasm_f32x4_sub(a, b);
#else
    return (SkaSimdF4){.v = {a.v[0] - b.v[0], a.v[1] - b.v[1], a.v[2] - b.v[2], a.v[3] - b.v[3]}};
#endif
}

SkaSimdF4 ska_simd_f4_mul(SkaSimdF4 a, SkaSimdF4 b) {
#if defined(SKA_SIMD_SSE2)
    return _mm_mul_ps(a, b);
#elif defined(SKA_SIMD_NEON)
    return vmulq_f32(a, b);
#elif defined(SKA_SIMD_WASM)
    return wasm_f32x4_mul(a, b);
#else
    return (SkaSimdF4){.v = {a.v[0] * b.v[0], a.v[1] * b.v[1], a.v[2] * b.v[2], a.v[3] * b.v[3]}};
#endif
}

SkaSimdF4 ska_simd_f4_fmadd(SkaSimdF4 a, SkaSimdF4 b, SkaSimdF4 c) {
#if defined(SKA_SIMD_NEON)
    return vfmaq_f32(c, a, b);
#else
    // SSE2 has no FMA, and baseline WASM SIMD128 has no fused multiply-add either
    // (that's part of the separate, newer "relaxed SIMD" proposal) -- compose it.
    return ska_simd_f4_add(ska_simd_f4_mul(a, b), c);
#endif
}

SkaSimdF4 ska_simd_f4_div(SkaSimdF4 a, SkaSimdF4 b) {
#if defined(SKA_SIMD_SSE2)
    return _mm_div_ps(a, b);
#elif defined(SKA_SIMD_NEON)
    return vdivq_f32(a, b);
#elif defined(SKA_SIMD_WASM)
    return wasm_f32x4_div(a, b);
#else
    return (SkaSimdF4){.v = {a.v[0] / b.v[0], a.v[1] / b.v[1], a.v[2] / b.v[2], a.v[3] / b.v[3]}};
#endif
}
