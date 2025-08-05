// mathcore.h
//
#ifndef _SUS_MATH_CORE_
#define _SUS_MATH_CORE_

#include "core.h"
#include "mathdef.h"

extern unsigned int usm_random_seed;
extern const float USM_SIN_TABLE[USM_TABLE_SIZE];
extern const float USM_COS_TABLE[USM_TABLE_SIZE];

// -----------------------------------------------------------------------------------------------------
//
//                              Embedded mathematical functions
// 
// -----------------------------------------------------------------------------------------------------

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

// Get a random number
SUS_INLINE sus_u32 SUSAPI sus_rand() {
    return usm_random_seed = (USM_RAND_A * usm_random_seed + USM_RAND_C) % (USM_RAND_M);
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
    return angle * USM_PI / 180.0f;
}
// The remainder of the division
SUS_INLINE sus_f32 SUSAPI sus_fmod(sus_f32 a, sus_f32 b) {
    sus_f32 quotient = a / b;
    return (quotient >= 0) ? (a - sus_floor(quotient) * b) : (a - sus_ceil(quotient) * b);
}
// Tabular sine
SUS_INLINE sus_f32 SUSAPI sus_sin(sus_i32 degrees) {
    return USM_SIN_TABLE[degrees % USM_TABLE_SIZE];
}
// Tabular cosine
SUS_INLINE sus_f32 SUSAPI sus_cos(sus_i32 degrees) {
    return USM_COS_TABLE[degrees % USM_TABLE_SIZE];
}

// Get the square root of a number
SUS_INLINE sus_f32 SUSAPI sus_sqrt(sus_f32 x) {
    if (x <= 0.0f) return 0.0f;
    sus_f32 guess = x;
    sus_f32 next_guess;
    for (int i = 0; i < USM_MAX_ITER; i++) {
        next_guess = (guess + x / guess) / 2.0f;
        if (sus_fabs(next_guess - guess) < USM_EPSILON) {
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

#endif /* !_UMATH_CORE_ */