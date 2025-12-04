// tmath.h
//
#ifndef _SUS_TMATH_
#define _SUS_TMATH_

////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4201)

// Two-dimensional vector
typedef union sus_vec2 {
	struct { sus_float x, y; };
	sus_float p[2];
} SUS_VEC2, * SUS_LPVEC2;
// Three-dimensional vector
typedef union sus_vec3 {
	struct { sus_float x, y, z; };
	struct { sus_float r, g, b; };
	sus_float p[3];
} SUS_VEC3, * SUS_LPVEC3;
// Four-dimensional vector
typedef union sus_vec4 {
	struct { sus_float x, y, z, w; };
	struct { sus_float r, g, b, a; };
	sus_float p[4];
} SUS_VEC4, * SUS_LPVEC4;
// Quaternion
typedef union sus_quat {
	struct { sus_float x, y, z, w; };
	sus_float p[4];
} SUS_QUAT, *SUS_LPQUAT;
// The 3x3 matrix
typedef union sus_mat3 {
	sus_float	m[3][3];
	sus_float	p[9];
	SUS_VEC3	v[3];
} SUS_MAT3, * SUS_LPMAT3;
// The 4x4 matrix
typedef union sus_mat4 {
	sus_float	m[4][4];
	sus_float	p[16];
	SUS_VEC4	v[4];
} SUS_MAT4, * SUS_LPMAT4;

#pragma warning(pop)

// -----------------------------------------------

////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------

// The sum of the vectors
SUS_INLINE SUS_VEC2 SUSAPI susVec2Sum(_In_ const SUS_VEC2 a, _In_ const SUS_VEC2 b) {
	return (SUS_VEC2) { a.x + b.x, a.y + b.y };
}
// The difference between two vectors
SUS_INLINE SUS_VEC2 SUSAPI susVec2Sub(_In_ const SUS_VEC2 a, _In_ const SUS_VEC2 b) {
	return (SUS_VEC2) { a.x - b.x, a.y - b.y };
}
// Scalar vector multiplication
SUS_INLINE SUS_VEC2 SUSAPI susVec2Scale(_In_ const SUS_VEC2 v, _In_ const sus_float s) {
	return (SUS_VEC2) { v.x* s, v.y* s };
}
// Multiplying a vector by a vector
SUS_INLINE SUS_VEC2 SUSAPI susVec2Mult(_In_ const SUS_VEC2 a, _In_ const SUS_VEC2 b) {
	return (SUS_VEC2) { a.x* b.x, a.y* b.y };
}
// Scalar product of vectors
SUS_INLINE sus_float SUSAPI susVec2Dot(_In_ const SUS_VEC2 a, _In_ const SUS_VEC2 b) {
	return a.x * b.x + a.y * b.y;
}
// Vector length
SUS_INLINE sus_float SUSAPI susVec2Length(_In_ const SUS_VEC2 v) {
	return sus_sqrt(susVec2Dot(v, v));
}
// Normalization of the vector
SUS_INLINE SUS_VEC2 SUSAPI susVec2Normalize(_In_ const SUS_VEC2 v) {
	sus_float len = susVec2Length(v);
	return len > 0.0f ? susVec2Scale(v, 1.0f / len) : v;
}

// -----------------------------------------------

// The sum of the vectors
SUS_INLINE SUS_VEC3 SUSAPI susVec3Sum(_In_ const SUS_VEC3 a, _In_ const SUS_VEC3 b) {
	return (SUS_VEC3) { a.x + b.x, a.y + b.y, a.z + b.z };
}
// The difference between two vectors
SUS_INLINE SUS_VEC3 SUSAPI susVec3Sub(_In_ const SUS_VEC3 a, _In_ const SUS_VEC3 b) {
	return (SUS_VEC3) { a.x - b.x, a.y - b.y, a.z - b.z };
}
// Scalar vector multiplication
SUS_INLINE SUS_VEC3 SUSAPI susVec3Scale(_In_ const SUS_VEC3 v, _In_ const sus_float s) {
	return (SUS_VEC3) { v.x* s, v.y* s, v.z* s };
}
// Multiplying a vector by a vector
SUS_INLINE SUS_VEC3 SUSAPI susVec3Mult(_In_ const SUS_VEC3 a, _In_ const SUS_VEC3 b) {
	return (SUS_VEC3) { a.x* b.x, a.y* b.y, a.z* b.z };
}
// Scalar product of vectors
SUS_INLINE sus_float SUSAPI susVec3Dot(_In_ const SUS_VEC3 a, _In_ const SUS_VEC3 b) {
	return a.x * b.x + a.y * b.y + a.z * b.z;
}
// Vector length
SUS_INLINE sus_float SUSAPI susVec3Length(_In_ const SUS_VEC3 v) {
	return sus_sqrt(susVec3Dot(v, v));
}
// Vector distance
SUS_INLINE sus_float SUSAPI susVec3Distance(_In_ const SUS_VEC3 a, _In_ const SUS_VEC3 b) {
	return susVec3Length(susVec3Sub(a, b));
}
// Normalization of the vector
SUS_INLINE SUS_VEC3 SUSAPI susVec3Normalize(_In_ const SUS_VEC3 v) {
	sus_float len = susVec3Length(v);
	return len > SUS_EPSILON ? susVec3Scale(v, 1.0f / len) : v;
}
// Linear interpolation
SUS_INLINE SUS_VEC3 SUSAPI susVec3Lerp(_In_ const SUS_VEC3 a, _In_ const SUS_VEC3 b, _In_ const sus_float t) {
	return susVec3Sum(a, susVec3Scale(susVec3Sub(b, a), t));
}
// Get the reflex vector
SUS_INLINE SUS_VEC3 SUSAPI susVec3Reflect(_In_ const SUS_VEC3 incident, _In_ const SUS_VEC3 normal) {
	return susVec3Sub(incident, susVec3Scale(normal, 2.0f * susVec3Dot(incident, normal)));
}
// Normalization of the vector
SUS_INLINE SUS_VEC3 SUSAPI susVec3Cross(_In_ const SUS_VEC3 a, _In_ const SUS_VEC3 b) {
	return (SUS_VEC3) {
		a.y* b.z - a.z * b.y,
		a.z* b.x - a.x * b.z,
		a.x* b.y - a.y * b.x
	};
}
// Convert a 4d vector to a öd vector
SUS_INLINE SUS_VEC4 SUSAPI susVec3ToVec4(_In_ const SUS_VEC3 v, _In_ const sus_float w) {
	return (SUS_VEC4) { v.x, v.y, v.z, w };
}
// Convert a 4d vector to a 3d vector
SUS_INLINE SUS_VEC3 SUSAPI susVec4ToVec3(_In_ const SUS_VEC4 v) {
	return sus_fabs(v.w) > SUS_EPSILON && v.w != 1.0f ? (SUS_VEC3) { v.x / v.w, v.y / v.w, v.z / v.w } : (SUS_VEC3) { v.x, v.y, v.z };
}

// -----------------------------------------------

// The scalar matrix
SUS_INLINE SUS_MAT4 SUSAPI susMat4Scale(_In_ SUS_VEC3 scale) {
	return (SUS_MAT4) {
		.m = {
			scale.x, 0.0f, 0.0f, 0.0f,
			0.0f, scale.y, 0.0f, 0.0f,
			0.0f, 0.0f, scale.z, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		}
	};
}
// Create a unit matrix
SUS_INLINE SUS_MAT4 SUSAPI susMat4Identity() {
	return susMat4Scale((SUS_VEC3) { 1.0f, 1.0f, 1.0f });
}
// Convert to a matrix from a three-dimensional vector
SUS_MAT4 SUSAPI susMat4Translate(
	_In_ SUS_VEC3 translation
);
// Matrix multiplication
SUS_MAT4 SUSAPI susMat4Mult(
	_In_ SUS_MAT4 a, _In_ SUS_MAT4 b
);
// Matrix multiplication
SUS_VEC4 SUSAPI susMat4MultVec4(
	_In_ SUS_MAT4 m, _In_ SUS_VEC4 v
);
// Rotating the matrix around the X axis
SUS_MAT4 SUSAPI susMat4RotateX(
	_In_ sus_int angle
);
// Rotating the matrix around the Y axis
SUS_MAT4 SUSAPI susMat4RotateY(
	_In_ sus_int angle
);
// Rotating the matrix around the Z axis
SUS_MAT4 SUSAPI susMat4RotateZ(
	_In_ sus_int angle
);
// Perspective projection
SUS_MAT4 SUSAPI susMat4Perspective(
	_In_ sus_uint fov, _In_ sus_float aspect,
	_In_ sus_float nearp, _In_ sus_float farp
);
// Orthographic projection
SUS_MAT4 SUSAPI susMat4Ortho(
	_In_ sus_float left, _In_ sus_float top,
	_In_ sus_float right, _In_ sus_float bottom,
	_In_ sus_float nearp, _In_ sus_float farp
);

// -----------------------------------------------

////////////////////////////////////////////////////////////////////////////////

#endif // !_SUS_TMATH_
