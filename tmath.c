// tmath.c
//
#include "include/susfwk/core.h"
#include "include/susfwk/math.h"
#include "include/susfwk/tmath.h"

////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------

// The scalar matrix
SUS_MAT4 SUSAPI susMat4Scale(_In_ SUS_VEC3 scale) {
	return (SUS_MAT4) {
		.m = {
			scale.x, 0.0f, 0.0f, 0.0f,
			0.0f, scale.y, 0.0f, 0.0f,
			0.0f, 0.0f, scale.z, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		}
	};
}
// Convert to a matrix from a three-dimensional vector
SUS_MAT4 SUSAPI susMat4Translate(_In_ SUS_VEC3 translation)
{
	SUS_MAT4 m = susMat4Identity();
	m.m[3][0] = translation.x;
	m.m[3][1] = translation.y;
	m.m[3][2] = translation.z;
	return m;
}
// Matrix multiplication
SUS_MAT4 SUSAPI susMat4Mult(_In_ SUS_MAT4 a, _In_ SUS_MAT4 b)
{
	SUS_MAT4 result = { 0 };
	for (int col = 0; col < 4; col++) {
		result.m[col][0] = a.m[0][0] * b.m[col][0] + a.m[1][0] * b.m[col][1] + a.m[2][0] * b.m[col][2] + a.m[3][0] * b.m[col][3];
		result.m[col][1] = a.m[0][1] * b.m[col][0] + a.m[1][1] * b.m[col][1] + a.m[2][1] * b.m[col][2] + a.m[3][1] * b.m[col][3];
		result.m[col][2] = a.m[0][2] * b.m[col][0] + a.m[1][2] * b.m[col][1] + a.m[2][2] * b.m[col][2] + a.m[3][2] * b.m[col][3];
		result.m[col][3] = a.m[0][3] * b.m[col][0] + a.m[1][3] * b.m[col][1] + a.m[2][3] * b.m[col][2] + a.m[3][3] * b.m[col][3];
	}
	return result;
}
// Matrix multiplication
SUS_VEC4 SUSAPI susMat4MultVec4(_In_ SUS_MAT4 m, _In_ SUS_VEC4 v)
{
	return (SUS_VEC4) {
        m.m[0][0] * v.x + m.m[0][1] * v.y + m.m[0][2] * v.z + m.m[0][3] * v.w,
        m.m[1][0] * v.x + m.m[1][1] * v.y + m.m[1][2] * v.z + m.m[1][3] * v.w,
        m.m[2][0] * v.x + m.m[2][1] * v.y + m.m[2][2] * v.z + m.m[2][3] * v.w,
        m.m[3][0] * v.x + m.m[3][1] * v.y + m.m[3][2] * v.z + m.m[3][3] * v.w
    };
}
// Rotating the matrix around the X axis
SUS_MAT4 SUSAPI susMat4RotateX(_In_ sus_int_t angle)
{
	sus_float_t c = sus_cos(angle);
	sus_float_t s = sus_sin(angle);
	SUS_MAT4 mat = susMat4Identity();
	mat.m[1][1] = c;
	mat.m[2][1] = s;
	mat.m[1][2] = -s;
	mat.m[2][2] = c;
	return mat;
}
// Rotating the matrix around the T axis
SUS_MAT4 SUSAPI susMat4RotateY(_In_ sus_int_t angle)
{
	sus_float_t c = sus_cos(angle);
	sus_float_t s = sus_sin(angle);
	SUS_MAT4 mat = susMat4Identity();
	mat.m[0][0] = c;
	mat.m[2][0] = s;
	mat.m[0][2] = -s;
	mat.m[2][2] = c;
	return mat;
}
// Rotating the matrix around the Z axis
SUS_MAT4 SUSAPI susMat4RotateZ(_In_ sus_int_t angle)
{
	sus_float_t c = sus_cos(angle);
	sus_float_t s = sus_sin(angle);
	SUS_MAT4 mat = susMat4Identity();
	mat.m[0][0] = c;
	mat.m[1][0] = s;
	mat.m[0][1] = -s;
	mat.m[1][1] = c;
	return mat;
}
// Create a rotation matrix along an arbitrary axis
SUS_MAT4 SUSAPI susMat4Rotate(_In_ sus_int_t angle, _In_ SUS_VEC3 a)
{
	a = susVec3Normalize(a);
	sus_float_t c = sus_cos(angle);
	sus_float_t s = sus_sin(angle);
	sus_float_t t = 1.0f - c;
	return (SUS_MAT4) {
		.m = {
			t * a.x * a.x + c,      t * a.x * a.y + s * a.z,  t * a.x * a.z - s * a.y,  0.0f,
			t * a.x * a.y - s * a.z, t * a.y * a.y + c,       t * a.y * a.z + s * a.x,  0.0f,
			t * a.x * a.z + s * a.y, t * a.y * a.z - s * a.x, t * a.z * a.z + c,        0.0f,
			0.0f,                    0.0f,                    0.0f,                    1.0f
		}
	};
}
// create an overview matrix
SUS_MAT4 SUSAPI susMat4LookAt(_In_ SUS_VEC3 eye, _In_ SUS_VEC3 target, _In_ SUS_VEC3 up)
{
	SUS_VEC3 f = susVec3Normalize(susVec3Sub(eye, target));
	SUS_VEC3 r = susVec3Normalize(susVec3Cross(susVec3Normalize(up), f));
	SUS_VEC3 u = susVec3Cross(f, r);
	return (SUS_MAT4) {
		.m = {
			r.x, u.x, f.x, 0.0f,
			r.y, u.y, f.y, 0.0f,
			r.z, u.z, f.z, 0.0f,
			-susVec3Dot(r, eye), -susVec3Dot(u, eye), -susVec3Dot(f, eye), 1.0f
		}
	};
}
// Perspective projection
SUS_MAT4 SUSAPI susMat4Perspective(_In_ sus_uint_t fov, _In_ sus_float_t aspect, _In_ sus_float_t nearp, _In_ sus_float_t farp)
{
	sus_float_t tan = sus_tan(fov / 2);
	return (SUS_MAT4) {
		.m = {
			1.0f / (aspect * tan), 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f / tan, 0.0f, 0.0f,
			0.0f, 0.0f, -(farp + nearp) / (farp - nearp), -1.0f,
			0.0f, 0.0f, -(2.0f * farp * nearp) / (farp - nearp), 0.0f
		}
	};
}
// Orographic projection
SUS_MAT4 SUSAPI susMat4Ortho(_In_ sus_float_t left, _In_ sus_float_t top, _In_ sus_float_t right, _In_ sus_float_t bottom, _In_ sus_float_t nearp, _In_ sus_float_t farp)
{
	return (SUS_MAT4) {
		.m = {
			2.0f / (right - left), 0.0f, 0.0f, 0.0f,
			0.0f, 2.0f / (top - bottom), 0.0f, 0.0f,
			0.0f, 0.0f, 2.0f / (nearp - farp), 0.0f,
			(right + left) / (left - right), (top + bottom) / (bottom - top), (farp + nearp) / (nearp - farp), 1.0f
		}
	};
}

// -----------------------------------------------

////////////////////////////////////////////////////////////////////////////////
