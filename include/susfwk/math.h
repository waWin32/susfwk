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
SUS_INLINE sus_int SUSAPI sus_floor(sus_float x) {
    return (x >= 0.0f) ? (sus_int)x : (sus_int)(x - 1);
}
// Rounding up
SUS_INLINE sus_int SUSAPI sus_ceil(sus_float x) {
    return (x > 0.0f) ? (sus_int)(x + 1) : (sus_int)x;
}
// The number module
SUS_INLINE sus_int SUSAPI sus_abs(sus_int x) {
    return (x < 0) ? -x : x;
}
// The module of a floating-point number
SUS_INLINE sus_double SUSAPI sus_fabs(sus_double x) {
    return (x < 0.0f) ? -x : x;
}
// The module of a floating-point number
SUS_INLINE sus_float SUSAPI sus_fabsf(sus_float x) {
    return (x < 0.0f) ? -x : x;
}
// Converting degrees to radians
SUS_INLINE sus_float SUSAPI sus_radians(sus_float angle) {
    return angle * SUS_PI / 180.0f;
}
// The remainder of the division
SUS_INLINE sus_float SUSAPI sus_fmodf(sus_float a, sus_float b) {
    sus_float quotient = a / b;
    return (quotient >= 0) ? (a - sus_floor(quotient) * b) : (a - sus_ceil(quotient) * b);
}
// Tabular sine
SUS_INLINE sus_float SUSAPI sus_sin(sus_int degrees) {
    degrees %= 360;
    if (degrees < 0) degrees += 360;
    if (degrees <= 90) return SUS_COS_TABLE[90 - degrees];
    if (degrees <= 180) return SUS_COS_TABLE[degrees - 90];
    if (degrees <= 270) return -SUS_COS_TABLE[270 - degrees];
    return -SUS_COS_TABLE[degrees - 270];
}
// Tabular cosine
SUS_INLINE sus_float SUSAPI sus_cos(sus_int degrees) {
    degrees %= 360;
    if (degrees < 0) degrees += 360;
    if (degrees <= 90) return SUS_COS_TABLE[degrees];
    if (degrees <= 180) return -SUS_COS_TABLE[180 - degrees];
    if (degrees <= 270) return -SUS_COS_TABLE[degrees - 180];
    return SUS_COS_TABLE[360 - degrees];
}
// Tabular acosine
SUS_INLINE sus_float SUSAPI sus_acos(sus_int x) {
    if (x >= 1.0f) return 0.0f;
    if (x <= -1.0f) return 180.0f;
    sus_float sign = 1.0f;
    if (x < 0.0f) { x = -x; sign = -1.0f; };
    sus_int i = 0;
    while (i < 90 && SUS_COS_TABLE[i] > x) i++;
    if (i == 0) return 0;
    return sign * ((i - 1) + (x - SUS_COS_TABLE[i]) / (SUS_COS_TABLE[i - 1] - SUS_COS_TABLE[i]));
}
// Tabular asine
SUS_INLINE sus_float SUSAPI sus_asin(sus_int x) {
    return 90.0f - sus_acos(sus_clamp(x, -1, 1));
}
// Get the tangent of the angle
SUS_INLINE sus_float SUSAPI sus_tan(sus_int x) {
    return sus_sin(x) / sus_cos(x);
}
// Get the square root of a number
SUS_INLINE sus_float SUSAPI sus_sqrt(sus_float x) {
    if (x <= 0.0f) return 0.0f;
    sus_float guess = x;
    sus_float next_guess;
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
SUS_INLINE sus_float SUSAPI sus_pow(sus_float x, sus_int exp) {
    if (exp < 0) return 1.0f / sus_pow(x, -exp);
    sus_float rez = 1.0f;
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
SUS_INLINE sus_float SUSAPI sus_atan(sus_float x) {
    sus_float x2 = x * x;
    return x * (0.7854f - 0.0804f * x2) / (1.0f - 0.825f * x2);
}
// Arctangent2 using for the full range of angles
SUS_INLINE sus_float SUSAPI sus_atan2(sus_float y, sus_float x) {
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