// tmath.h
//
#ifndef _MATH_VECTORS_
#define _MATH_VECTORS_

#include "core.h"
#include "mathcore.h"

#define SUSAPITM __vectorcall

//////////////////////////////////////////////////
//												//
//					2D Vectors					//
//												//
//////////////////////////////////////////////////

// 2D vector
typedef struct Vec2 {
	sus_float x;
	sus_float y;
} SUS_VEC2, *PSUS_VEC2, *LPSUS_VEC2;

// The sum of two 2D vectors
SUS_INLINE SUS_VEC2 SUSAPITM vSum2(SUS_VEC2 v1, const SUS_VEC2 v2) {
	return (SUS_VEC2) { v1.x + v2.x, v1.y + v2.y };
}
// Subtract two 2D vectors
SUS_INLINE SUS_VEC2 SUSAPITM vSub2(SUS_VEC2 v1, const SUS_VEC2 v2) {
	return (SUS_VEC2) { v1.x - v2.x, v1.y - v2.y };
}
// The multiply of two 2D vectors
SUS_INLINE SUS_VEC2 SUSAPITM vMult2(SUS_VEC2 v1, const SUS_VEC2 v2) {
	return (SUS_VEC2) { v1.x* v2.x, v1.y* v2.y };
}
// divide two 2D vectors
SUS_INLINE SUS_VEC2 SUSAPITM vDiv2(SUS_VEC2 v1, const SUS_VEC2 v2) {
	return (SUS_VEC2) { v1.x / v2.x, v1.y / v2.y };
}
// Multiplying a 2D vector by a scalar
SUS_INLINE SUS_VEC2 SUSAPITM sMult2(SUS_VEC2 v, const sus_float s) {
	return (SUS_VEC2) { v.x* s, v.y* s };
}
// Dividing a 2D vector by a scalar
SUS_INLINE SUS_VEC2 SUSAPITM sDiv2(SUS_VEC2 v, const sus_float s) {
	return (SUS_VEC2) { v.x / s, v.y / s };
}

//////////////////////////////////////////////////
//												//
//					3D Vectors					//
//												//
//////////////////////////////////////////////////

// 3D vector
typedef struct Vec3 {
	sus_float x;
	sus_float y;
	sus_float z;
} SUS_VEC3, *PSUS_VEC3, *LPSUS_VEC3;

// The sum of two 3D vectors
SUS_INLINE SUS_VEC3 SUSAPITM vSum3(SUS_VEC3 v1, const SUS_VEC3 v2) {
	return (SUS_VEC3) { v1.x + v2.x, v1.y + v2.y, v1.z + v2.z };
}
// Subtract two 3D vectors
SUS_INLINE SUS_VEC3 SUSAPITM vSub3(SUS_VEC3 v1, const SUS_VEC3 v2) {
	return (SUS_VEC3) { v1.x - v2.x, v1.y - v2.y, v1.z - v2.z };
}
// The multiply of two 3D vectors
SUS_INLINE SUS_VEC3 SUSAPITM vMult3(SUS_VEC3 v1, const SUS_VEC3 v2) {
	return (SUS_VEC3) { v1.x* v2.x, v1.y* v2.y, v1.z* v2.z };
}
// Multiplying a 3D vector by a scalar
SUS_INLINE SUS_VEC3 SUSAPITM sMult3(SUS_VEC3 v, const sus_float s) {
	return (SUS_VEC3) { v.x* s, v.y* s, v.z * s };
}
// divide two 3D vectors
SUS_INLINE SUS_VEC3 SUSAPITM vDiv3(SUS_VEC3 v1, const SUS_VEC3 v2) {
	return (SUS_VEC3) { v1.x / v2.x, v1.y / v2.y, v1.z / v2.z };
}
// Dividing a 3D vector by a scalar
SUS_INLINE SUS_VEC3 SUSAPITM sDiv3(SUS_VEC3 v, const sus_float s) {
	return (SUS_VEC3) { v.x / s, v.y / s, v.z / s };
}

// The scalar product of two 3D vectors
SUS_INLINE float SUSAPITM vDot3(SUS_VEC3 v1, const SUS_VEC3 v2) {
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}
// Calculating the length of a vector
SUS_INLINE float SUSAPITM vLength3(const SUS_VEC3 v) {
	return (float)sus_sqrt(vDot3(v, v));
}
// Normalization of the 3D vector
SUS_INLINE SUS_VEC3 SUSAPITM vNormalize3(const SUS_VEC3 v) {
	return sDiv3(v, vLength3(v));
}
// Calculating the distance from point to point of a vector
SUS_INLINE float SUSAPITM vDistance3(const SUS_VEC3 v1, const SUS_VEC3 v2) {
	return vLength3(vSub3(v1, v2));
}
// Getting the normal of the vector
SUS_INLINE SUS_VEC3 SUSAPITM vGetNormal3(const SUS_VEC3 v1, const SUS_VEC3 v2) {
	return vNormalize3((SUS_VEC3) {
		v1.y * v2.z - v1.z * v2.y,
		v1.z * v2.x - v1.x * v2.z,
		v1.x * v2.y - v1.y * v2.x
	});
}
// Get the modulus from a vector
SUS_INLINE SUS_VEC3 SUSAPITM vAbs3(SUS_VEC3 v) {
	return (SUS_VEC3) { sus_fabs(v.x), sus_fabs(v.y), sus_fabs(v.z) };
}
// Rotate around the Y axis
SUS_INLINE SUS_VEC3 SUSAPITM sRotateXaxis(sus_float angle) {
	return (SUS_VEC3) { 0, sus_cos((int)angle), sus_sin((int)angle) };
}
// Rotate around the Y axis
SUS_INLINE SUS_VEC3 SUSAPITM sRotateYaxis(sus_float angle) {
	return (SUS_VEC3) { sus_cos((int)angle), 0, sus_sin((int)angle) };
}
// Rotate around the Y axis
SUS_INLINE SUS_VEC3 SUSAPITM sRotateZaxis(sus_float angle) {
	return (SUS_VEC3) { sus_cos((int)angle), sus_sin((int)angle), 0 };
}
// Limiting a vector to its components
SUS_INLINE SUS_VEC3 SUSAPITM vClamp3(SUS_VEC3 v, SUS_VEC3 vmin, SUS_VEC3 vmax) {
	return (SUS_VEC3) {
		max(vmin.x, min(v.x, vmax.x)),
		max(vmin.y, min(v.y, vmax.y)),
		max(vmin.z, min(v.z, vmax.z))
	};
}

//////////////////////////////////////////////////
//												//
//					4D Vectors					//
//												//
//////////////////////////////////////////////////

// 4D vector
typedef struct vec4 {
	sus_float x;
	sus_float y;
	sus_float z;
	sus_float w;
} VEC4, *PVEC4, *LPVEC4;

#endif /* !_MATH_VECTORS_ */
