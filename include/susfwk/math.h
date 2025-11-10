// mathcore.h
//
#ifndef _SUS_MATH_CORE_
#define _SUS_MATH_CORE_

#include "core.h"

#define SUS_PI				3.1415926f
#define SUS_EPSILON			0.001f
#define SUS_MAX_ITER_SQRT	10
#define SUS_RAND_SEED		3157
#define SUS_RAND_A			1664525
#define SUS_RAND_C			1013904223
#define SUS_RAND_M			1U << 31

extern sus_u32 usm_random_seed;
extern const sus_float SUS_COS_TABLE[];

// -----------------------------------------------------------------------------------------------------
//
//                              Embedded mathematical functions
// 
// -----------------------------------------------------------------------------------------------------

#define sus_max(a,b)            (((a) > (b)) ? (a) : (b))
#define sus_min(a,b)            (((a) < (b)) ? (a) : (b))
#define sus_clamp(value, minv, maxv) sus_max(minv, sus_min(maxv, value))

// Get a random number
SUS_INLINE sus_u32 SUSAPI sus_rand() {
    return usm_random_seed = (SUS_RAND_A * usm_random_seed + SUS_RAND_C) % (SUS_RAND_M);
}
// Set the value of the seed random
SUS_INLINE VOID SUSAPI sus_srand(sus_u32 seed) {
    usm_random_seed = seed;
}

#define sus_randint(a, b) ((sus_rand() % ((b) - (a) + 1)) + (a))

// Rounding down
SUS_INLINE sus_i32 SUSAPI sus_floor(sus_f32 x) {
    return (x >= 0.0f) ? (sus_i32)x : (sus_i32)(x - 1);
}
// Rounding up
SUS_INLINE sus_i32 SUSAPI sus_ceil(sus_f64 x) {
    return (x > 0.0f) ? (sus_i32)(x + 1) : (sus_i32)x;
}
// The number module
SUS_INLINE sus_i32 SUSAPI sus_abs(sus_i32 x) {
    return (x < 0) ? -x : x;
}
// The module of a floating-point number
SUS_INLINE sus_f32 SUSAPI sus_fabs(sus_f32 x) {
    return (x < 0.0f) ? -x : x;
}
// The module of a floating-point number
SUS_INLINE sus_f64 SUSAPI sus_dabs(sus_f64 x) {
    return (x < 0.0f) ? -x : x;
}
// Converting degrees to radians
SUS_INLINE sus_f32 SUSAPI sus_radians(sus_f32 angle) {
    return angle * SUS_PI / 180.0f;
}
// The remainder of the division
SUS_INLINE sus_f32 SUSAPI sus_fmod(sus_f32 a, sus_f32 b) {
    sus_f32 quotient = a / b;
    return (quotient >= 0) ? (a - sus_floor(quotient) * b) : (a - sus_ceil(quotient) * b);
}
// Tabular sine
SUS_INLINE sus_f32 SUSAPI sus_sin(sus_i32 degrees) {
    degrees %= 360;
    if (degrees < 0) degrees += 360;
    if (degrees <= 90) return SUS_COS_TABLE[90 - degrees];
    if (degrees <= 180) return SUS_COS_TABLE[degrees - 90];
    if (degrees <= 270) return -SUS_COS_TABLE[270 - degrees];
    return -SUS_COS_TABLE[degrees - 270];
}
// Tabular cosine
SUS_INLINE sus_f32 SUSAPI sus_cos(sus_i32 degrees) {
    degrees %= 360;
    if (degrees < 0) degrees += 360;
    if (degrees <= 90) return SUS_COS_TABLE[degrees];
    if (degrees <= 180) return -SUS_COS_TABLE[180 - degrees];
    if (degrees <= 270) return -SUS_COS_TABLE[degrees - 180];
    return SUS_COS_TABLE[360 - degrees];
}
// Tabular acosine
SUS_INLINE sus_f32 SUSAPI sus_acos(sus_f32 x) {
    if (x >= 1.0f) return 0.0f;
    if (x <= -1.0f) return 180.0f;
    sus_f32 sign = 1.0f;
    if (x < 0.0f) { x = -x; sign = -1.0f; };
    sus_int i = 0;
    while (i < 90 && SUS_COS_TABLE[i] > x) i++;
    if (i == 0) return 0;
    return sign * ((i - 1) + (x - SUS_COS_TABLE[i]) / (SUS_COS_TABLE[i - 1] - SUS_COS_TABLE[i]));
}
// Tabular asine
SUS_INLINE sus_f32 SUSAPI sus_asin(sus_i32 x) {
    return 90.0f - sus_acos(sus_clamp(x, -1.0f, 1.0f));
}

// Get the square root of a number
SUS_INLINE sus_f32 SUSAPI sus_sqrt(sus_f32 x) {
    if (x <= 0.0f) return 0.0f;
    sus_f32 guess = x;
    sus_f32 next_guess;
    for (int i = 0; i < SUS_MAX_ITER_SQRT; i++) {
        next_guess = (guess + x / guess) / 2.0f;
        if (sus_fabs(next_guess - guess) < SUS_EPSILON) {
            break;
        }
        guess = next_guess;
    }
    return guess;
}
// Raise a number to a power
SUS_INLINE sus_f32 SUSAPI sus_pow(sus_f32 x, sus_i32 exp) {
    if (exp < 0) return 1.0f / sus_pow(x, -exp);
    sus_f32 rez = 1.0f;
    while (exp > 0) {
        if (exp % 2 == 1) {
            rez *= x;
        }
        x *= x;
        exp /= 2;
    }
    return rez;
}
// Arctangent using the approximation
SUS_INLINE sus_f32 SUSAPI sus_atan(sus_f32 x) {
    sus_f32 x2 = x * x;
    return x * (0.7854f - 0.0804f * x2) / (1.0f - 0.825f * x2);
}
// Arctangent2 using for the full range of angles
SUS_INLINE sus_f32 SUSAPI sus_atan2(sus_f32 y, sus_f32 x) {
    if (x == 0.0f) {
        if (y > 0.0f) return 90.0f;
        if (y < 0.0f) return -90.0f;
        return 0.0f;
    }
    sus_float atan = sus_atan(y / x);
    if (x < 0.0f) {
        if (y >= 0.0f) return atan + 180.0f;
        return atan - 180.0f;
    }
    return atan;
}

#endif /* !_UMATH_CORE_ */