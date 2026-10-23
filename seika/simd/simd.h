#pragma once

#include "seika/defines.h"

#ifdef __cplusplus
extern "C" {
#endif

// Portable 4-lane 32-bit vector: backed by SSE2 on x86-64, NEON on ARM64, WASM SIMD128 on
// Emscripten/wasm32 (only when enabled via -msimd128), and a plain scalar struct everywhere
// else. The backing type is typedef'd directly to the platform intrinsic type, so callers that
// need more than splat/store can still use raw intrinsics on it (e.g. _mm_add_epi32 on x86-64,
// vaddq_u32 on ARM64, wasm_i32x4_add on wasm32).
#if defined(__x86_64__) || defined(_M_X64)
#include <emmintrin.h>
#define SKA_SIMD_SSE2 1
typedef __m128i SkaSimdI4;
typedef __m128 SkaSimdF4;
#elif defined(__aarch64__) || defined(_M_ARM64)
#include <arm_neon.h>
#define SKA_SIMD_NEON 1
typedef uint32x4_t SkaSimdI4;
typedef float32x4_t SkaSimdF4;
#elif defined(__wasm_simd128__)
#include <wasm_simd128.h>
#define SKA_SIMD_WASM 1
typedef v128_t SkaSimdI4;
typedef v128_t SkaSimdF4;
#else
#define SKA_SIMD_SCALAR 1
typedef struct SkaSimdI4 { u32 v[4]; } SkaSimdI4;
typedef struct SkaSimdF4 { f32 v[4]; } SkaSimdF4;
#endif

#define SKA_SIMD_I4_LANES 4
#define SKA_SIMD_F4_LANES 4

SkaSimdI4 ska_simd_i4_splat(u32 value);
void ska_simd_i4_storeu(u32* dst, SkaSimdI4 vec);

// Fills 'count' consecutive 32-bit elements at 'dst' with 'pattern' (e.g. a packed RGBA8 pixel),
// using wide SIMD stores where available, with a scalar tail for any remainder.
void ska_simd_fill_u32(u32* dst, u32 pattern, usize count);

SkaSimdF4 ska_simd_f4_splat(f32 value);
SkaSimdF4 ska_simd_f4_loadu(const f32* src);
void ska_simd_f4_storeu(f32* dst, SkaSimdF4 vec);
SkaSimdF4 ska_simd_f4_add(SkaSimdF4 a, SkaSimdF4 b);
SkaSimdF4 ska_simd_f4_sub(SkaSimdF4 a, SkaSimdF4 b);
SkaSimdF4 ska_simd_f4_mul(SkaSimdF4 a, SkaSimdF4 b);
// (a * b) + c -- fused on NEON (single rounding), separate multiply+add on SSE2 (no FMA in baseline SSE2)
SkaSimdF4 ska_simd_f4_fmadd(SkaSimdF4 a, SkaSimdF4 b, SkaSimdF4 c);
SkaSimdF4 ska_simd_f4_div(SkaSimdF4 a, SkaSimdF4 b);

#ifdef __cplusplus
}
#endif
