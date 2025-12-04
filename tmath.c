// tmath.c
//
#include "include/susfwk/core.h"
#include "include/susfwk/math.h"
#include "include/susfwk/tmath.h"

////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------

// Convert to a matrix from a three-dimensional vector
SUS_MAT4 SUSAPI susMat4Translate(_In_ SUS_VEC3 translation)
{
	SUS_MAT4 m = susMat4Identity();
	m.m[0][3] = translation.x;
	m.m[1][3] = translation.y;
	m.m[2][3] = translation.z;
	return m;
}
// Matrix multiplication
SUS_MAT4 SUSAPI susMat4Mult(_In_ SUS_MAT4 a, _In_ SUS_MAT4 b)
{
	SUS_MAT4 result = { 0 };
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			sus_float sum = 0.0f;
			for (int k = 0; k < 4; k++) {
				sum += a.m[i][k] * b.m[k][j];
			}
			result.m[i][j] = sum;
		}
	}
	return result;
}
// Matrix multiplication
SUS_VEC4 SUSAPI susMat4MultVec4(_In_ SUS_MAT4 m, _In_ SUS_VEC4 v)
{
	return (SUS_VEC4) {
		m.m[0][0] * v.x + m.m[1][0] * v.y + m.m[0][2] * v.z + m.m[0][3] * v.w,
		m.m[1][0] * v.x + m.m[1][1] * v.y + m.m[1][2] * v.z + m.m[1][3] * v.w,
		m.m[2][0] * v.x + m.m[2][1] * v.y + m.m[2][2] * v.z + m.m[2][3] * v.w,
		m.m[3][0] * v.x + m.m[3][1] * v.y + m.m[3][2] * v.z + m.m[3][3] * v.w
	};
}
// Rotating the matrix around the X axis
SUS_MAT4 SUSAPI susMat4RotateX(_In_ sus_int angle)
{
	sus_float cosA = sus_cos(angle);
	sus_float sinA = sus_sin(angle);
	SUS_MAT4 mat = susMat4Identity();
	mat.m[1][1] = cosA;
	mat.m[1][2] = sinA;
	mat.m[2][1] = -sinA;
	mat.m[2][2] = cosA;
	return mat;
}
// Rotating the matrix around the T axis
SUS_MAT4 SUSAPI susMat4RotateY(_In_ sus_int angle)
{
	sus_float cosA = sus_cos(angle);
	sus_float sinA = sus_sin(angle);
	SUS_MAT4 mat = susMat4Identity();
	mat.m[0][0] = cosA;
	mat.m[0][2] = sinA;
	mat.m[2][0] = -sinA;
	mat.m[2][2] = cosA;
	return mat;
}
// Rotating the matrix around the Z axis
SUS_MAT4 SUSAPI susMat4RotateZ(_In_ sus_int angle)
{
	sus_float cosA = sus_cos(angle);
	sus_float sinA = sus_sin(angle);
	SUS_MAT4 mat = susMat4Identity();
	mat.m[0][0] = cosA;
	mat.m[0][1] = sinA;
	mat.m[1][0] = -sinA;
	mat.m[1][1] = cosA;
	return mat;
}

// Perspective projection
SUS_MAT4 SUSAPI susMat4Perspective(_In_ sus_uint fov, _In_ sus_float aspect, _In_ sus_float nearp, _In_ sus_float farp)
{
	sus_float tan = sus_tan(fov / 2);
	return (SUS_MAT4) {
		.m = {
			1.0f / (aspect * tan), 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f / tan, 0.0f, 0.0f,
			0.0f, 0.0f, (farp + nearp) / (nearp - farp), -1.0f,
			0.0f, 0.0f, (2.0f * farp * nearp) / (nearp - farp), 0.0f
		}
	};
}
// Orographic projection
SUS_MAT4 SUSAPI susMat4Ortho(_In_ sus_float left, _In_ sus_float top, _In_ sus_float right, _In_ sus_float bottom, _In_ sus_float nearp, _In_ sus_float farp)
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
