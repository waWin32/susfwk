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
	float x;
	float y;
} VEC2, *PVEC2, *LPVEC2;

// The sum of two 2D vectors
SUS_INLINE VEC2 SUSAPITM vSum2(VEC2 v1, const VEC2 v2) {
	return (VEC2) { v1.x + v2.x, v1.y + v2.y };
}
// Subtract two 2D vectors
SUS_INLINE VEC2 SUSAPITM vSub2(VEC2 v1, const VEC2 v2) {
	return (VEC2) { v1.x - v2.x, v1.y - v2.y };
}
// The multiply of two 2D vectors
SUS_INLINE VEC2 SUSAPITM vMult2(VEC2 v1, const VEC2 v2) {
	return (VEC2) { v1.x* v2.x, v1.y* v2.y };
}
// divide two 2D vectors
SUS_INLINE VEC2 SUSAPITM vDiv2(VEC2 v1, const VEC2 v2) {
	return (VEC2) { v1.x / v2.x, v1.y / v2.y };
}
// Multiplying a 2D vector by a scalar
SUS_INLINE VEC2 SUSAPITM sMult2(VEC2 v, const float s) {
	return (VEC2) { v.x* s, v.y* s };
}
// Dividing a 2D vector by a scalar
SUS_INLINE VEC2 SUSAPITM sDiv2(VEC2 v, const float s) {
	return (VEC2) { v.x / s, v.y / s };
}

//////////////////////////////////////////////////
//												//
//					3D Vectors					//
//												//
//////////////////////////////////////////////////

// 3D vector
typedef struct Vec3 {
	float x;
	float y;
	float z;
} VEC3, *PVEC3, *LPVEC3;

// The sum of two 3D vectors
SUS_INLINE VEC3 SUSAPITM vSum3(VEC3 v1, const VEC3 v2) {
	return (VEC3) { v1.x + v2.x, v1.y + v2.y, v1.z + v2.z };
}
// Subtract two 3D vectors
SUS_INLINE VEC3 SUSAPITM vSub3(VEC3 v1, const VEC3 v2) {
	return (VEC3) { v1.x - v2.x, v1.y - v2.y, v1.z - v2.z };
}
// The multiply of two 3D vectors
SUS_INLINE VEC3 SUSAPITM vMult3(VEC3 v1, const VEC3 v2) {
	return (VEC3) { v1.x* v2.x, v1.y* v2.y, v1.z* v2.z };
}
// Multiplying a 3D vector by a scalar
SUS_INLINE VEC3 SUSAPITM sMult3(VEC3 v, const float s) {
	return (VEC3) { v.x* s, v.y* s, v.z * s };
}
// divide two 3D vectors
SUS_INLINE VEC3 SUSAPITM vDiv3(VEC3 v1, const VEC3 v2) {
	return (VEC3) { v1.x / v2.x, v1.y / v2.y, v1.z / v2.z };
}
// Dividing a 3D vector by a scalar
SUS_INLINE VEC3 SUSAPITM sDiv3(VEC3 v, const float s) {
	return (VEC3) { v.x / s, v.y / s, v.z / s };
}

// The scalar product of two 3D vectors
SUS_INLINE float SUSAPITM vDot3(VEC3 v1, const VEC3 v2) {
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}
// Calculating the length of a vector
SUS_INLINE float SUSAPITM vLength3(const VEC3 v) {
	return (float)sus_sqrt(vDot3(v, v));
}
// Normalization of the 3D vector
SUS_INLINE VEC3 SUSAPITM vNormalize3(const VEC3 v) {
	return sDiv3(v, vLength3(v));
}
// Calculating the distance from point to point of a vector
SUS_INLINE float SUSAPITM vDistance3(const VEC3 v1, const VEC3 v2) {
	return vLength3(vSub3(v1, v2));
}
// Getting the normal of the vector
SUS_INLINE VEC3 SUSAPITM vGetNormal3(const VEC3 v1, const VEC3 v2) {
	return vNormalize3((VEC3) {
		v1.y * v2.z - v1.z * v2.y,
		v1.z * v2.x - v1.x * v2.z,
		v1.x * v2.y - v1.y * v2.x
	});
}
// Get the modulus from a vector
SUS_INLINE VEC3 SUSAPITM vAbs3(VEC3 v) {
	return (VEC3) { sus_fabs(v.x), sus_fabs(v.y), sus_fabs(v.z) };
}
// Rotate around the Y axis
SUS_INLINE VEC3 SUSAPITM sRotateXaxis(float angle) {
	return (VEC3) { 0, sus_cos((int)angle), sus_sin((int)angle) };
}
// Rotate around the Y axis
SUS_INLINE VEC3 SUSAPITM sRotateYaxis(float angle) {
	return (VEC3) { sus_cos((int)angle), 0, sus_sin((int)angle) };
}
// Rotate around the Y axis
SUS_INLINE VEC3 SUSAPITM sRotateZaxis(float angle) {
	return (VEC3) { sus_cos((int)angle), sus_sin((int)angle), 0 };
}
// Limiting a vector to its components
SUS_INLINE VEC3 SUSAPITM vClamp3(VEC3 v, VEC3 vmin, VEC3 vmax) {
	return (VEC3) {
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
typedef struct Vec4 {
	float x;
	float y;
	float z;
} VEC4, *PVEC4, *LPVEC4;

#endif /* !_MATH_VECTORS_ */
