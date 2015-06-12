#ifndef _MATHFUNC_H_
#define _MATHFUNC_H_

#include "Defines.h"


unsigned int Convert2D_1D(unsigned int x, unsigned int y, unsigned int width);

FLOAT4 Mult_Vertex3x3(FLOAT4 ver, MATRIX3X3 mat);

FLOAT4 Mult_Vertex4x4(FLOAT4 ver, MATRIX4X4 mat4);

FLOAT4 Negate_Vec3(FLOAT4 A);

MATRIX3X3 Transpose(MATRIX4X4 A);

MATRIX4X4 Identity();

MATRIX4X4 Mult_4x4(MATRIX4X4 A, MATRIX4X4 B);

MATRIX4X4 Scale_4x4(MATRIX4X4 A, float x, float y, float z);

MATRIX4X4 RotateX(MATRIX4X4 A, float radians);

MATRIX4X4 RotateY(MATRIX4X4 A, float radians);

MATRIX4X4 RotateZ(MATRIX4X4 A, float radians);

MATRIX4X4 RotateX_Local(MATRIX4X4 A, float radians);

MATRIX4X4 RotateY_Local(MATRIX4X4 A, float radians);

MATRIX4X4 RotateZ_Local(MATRIX4X4 A, float radians);

MATRIX4X4 Translate(MATRIX4X4 A, float x, float y, float z);

MATRIX4X4 FastInverse(MATRIX4X4 Mat);

MATRIX4X4 CreateProjectionMatrix(float zfar, float znear, unsigned int fov, float ar);

#endif