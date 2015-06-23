#include "MathFunc.h"


using namespace DirectX;


unsigned int Convert2D_1D(unsigned int x, unsigned int y, unsigned int width){
	return y * width + x;
}

FLOAT3 Mult_Vec3(FLOAT3 v, MATRIX3X3 m){
	FLOAT3 ans = {
		(m.a * v.x) + (m.b * v.y) + (m.c * v.z),
		(m.i * v.x) + (m.j * v.y) + (m.k * v.z),
		(m.x * v.x) + (m.y * v.y) + (m.z * v.z)
	};
	return ans;
}

FLOAT3 Subtract(FLOAT3 a, FLOAT3 b){
	FLOAT3 tmp;
	tmp.x = a.x - b.x;
	tmp.y = a.y - b.y;
	tmp.z = a.z - b.z;
	return tmp;
}

FLOAT4 Mult_Vertex3x3(FLOAT4 ver, MATRIX3X3 mat){
	FLOAT4 answer;
	answer.x = (ver.x * mat.a) + (ver.y * mat.i) + (ver.z * mat.x);
	answer.y = (ver.x * mat.b) + (ver.y * mat.j) + (ver.z * mat.y);
	answer.z = (ver.x * mat.c) + (ver.y * mat.k) + (ver.z * mat.z);
	return answer;
}

FLOAT4 Mult_Vertex4x4(FLOAT4 ver, MATRIX4X4 mat4){
	FLOAT4 answer;
	answer.x = (ver.x * mat4.a) + (ver.y * mat4.e) + (ver.z * mat4.i) + (ver.w * mat4.m);
	answer.y = (ver.x * mat4.b) + (ver.y * mat4.f) + (ver.z * mat4.j) + (ver.w * mat4.n);
	answer.z = (ver.x * mat4.c) + (ver.y * mat4.g) + (ver.z * mat4.k) + (ver.w * mat4.o);
	answer.w = (ver.x * mat4.d) + (ver.y * mat4.h) + (ver.z * mat4.l) + (ver.w * mat4.p);
	return answer;
}

FLOAT4 Subtract_F4(FLOAT4 A, FLOAT4 B){
	FLOAT4 out;
	out.x = A.x - B.x;
	out.y = A.y - B.y;
	out.z = A.z - B.z;
	out.w = A.w - B.w;
	return out;
}

MATRIX3X3 Mult_3x3(MATRIX3X3 A, MATRIX3X3 B){
	MATRIX3X3 ans;

	ans.a = (A.a * B.a) + (A.b * B.i) + (A.c * B.x);
	ans.b = (A.a * B.b) + (A.b * B.j) + (A.c * B.y);
	ans.c = (A.a * B.c) + (A.b * B.k) + (A.c * B.z);

	ans.i = (A.i * B.a) + (A.j * B.i) + (A.k * B.x);
	ans.j = (A.i * B.b) + (A.j * B.j) + (A.k * B.y);
	ans.k = (A.i * B.c) + (A.j * B.k) + (A.k * B.z);

	ans.x = (A.x * B.a) + (A.y * B.i) + (A.z * B.x);
	ans.y = (A.x * B.b) + (A.y * B.j) + (A.z * B.y);
	ans.z = (A.x * B.c) + (A.y * B.k) + (A.z * B.z);

	return ans;
}

MATRIX4X4 Mult_4x4(MATRIX4X4 A, MATRIX4X4 B){
	MATRIX4X4 ans;

	ans.a = (A.a * B.a) + (A.b * B.e) + (A.c * B.i) + (A.d * B.m);
	ans.b = (A.a * B.b) + (A.b * B.f) + (A.c * B.j) + (A.d * B.n);
	ans.c = (A.a * B.c) + (A.b * B.g) + (A.c * B.k) + (A.d * B.o);
	ans.d = (A.a * B.d) + (A.b * B.h) + (A.c * B.l) + (A.d * B.p);

	ans.e = (A.e * B.a) + (A.f * B.e) + (A.g * B.i) + (A.h * B.m);
	ans.f = (A.e * B.b) + (A.f * B.f) + (A.g * B.j) + (A.h * B.n);
	ans.g = (A.e * B.c) + (A.f * B.g) + (A.g * B.k) + (A.h * B.o);
	ans.h = (A.e * B.d) + (A.f * B.h) + (A.g * B.l) + (A.h * B.p);

	ans.i = (A.i * B.a) + (A.j * B.e) + (A.k * B.i) + (A.l * B.m);
	ans.j = (A.i * B.b) + (A.j * B.f) + (A.k * B.j) + (A.l * B.n);
	ans.k = (A.i * B.c) + (A.j * B.g) + (A.k * B.k) + (A.l * B.o);
	ans.l = (A.i * B.d) + (A.j * B.h) + (A.k * B.l) + (A.l * B.p);

	ans.m = (A.m * B.a) + (A.n * B.e) + (A.o * B.i) + (A.p * B.m);
	ans.n = (A.m * B.b) + (A.n * B.f) + (A.o * B.j) + (A.p * B.n);
	ans.o = (A.m * B.c) + (A.n * B.g) + (A.o * B.k) + (A.p * B.o);
	ans.p = (A.m * B.d) + (A.n * B.h) + (A.o * B.l) + (A.p * B.p);

	return ans;
}

MATRIX4X4 Scale_4x4(MATRIX4X4 A, float x, float y, float z){
	MATRIX4X4 scale = {
		x, 0, 0, 0,
		0, y, 0, 0,
		0, 0, z, 0,
		0, 0, 0, 1
	};
	return Mult_4x4(scale, A);
}

MATRIX4X4 Identity(){
	MATRIX4X4 A = {
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};
	return A;
}

MATRIX3X3 RotateX(MATRIX3X3 A, float radians){
	MATRIX3X3 rotate = {
		1.0f, 0.0f, 0.0f,
		0.0f, cos(radians), (-sin(radians)),
		0.0f, sin(radians), cos(radians)
	};

	return Mult_3x3(A, rotate);
}

MATRIX3X3 RotateY(MATRIX3X3 A, float radians){
	MATRIX3X3 rotate = {
		cos(radians), 0, sin(radians),
		0, 0, 0,
		-sin(radians), 0, cos(radians)
	};

	return Mult_3x3(A, rotate);
}

MATRIX3X3 RotateZ(MATRIX3X3 A, float radians){
	MATRIX3X3 rotate = {
		cos(radians), 0, sin(radians),
		0, 0, 0,
		-sin(radians), 0, cos(radians)
	};

	return Mult_3x3(A, rotate);
}

MATRIX4X4 RotateX(MATRIX4X4 A, float radians){
	MATRIX4X4 rotate = {
		1, 0, 0, 0,
		0, cos(radians), (-sin(radians)), 0,
		0, sin(radians), cos(radians), 0,
		0, 0, 0, 1
	};

	return Mult_4x4(A, rotate);
}

MATRIX4X4 RotateY(MATRIX4X4 A, float radians){
	MATRIX4X4 rotate = {
		cos(radians), 0, sin(radians), 0,
		0, 1, 0, 0,
		-sin(radians), 0, cos(radians), 0,
		0, 0, 0, 1 };

	return Mult_4x4(A, rotate);
}

MATRIX4X4 RotateZ(MATRIX4X4 A, float radians){
	MATRIX4X4 rotate = {
		cos(radians), 0, sin(radians), 0,
		0, 1, 0, 0,
		-sin(radians), 0, cos(radians), 0,
		0, 0, 0, 1 };

	return Mult_4x4(A, rotate);
}

MATRIX4X4 RotateX_Local(MATRIX4X4 A, float dt){
	float radians = 500.0f;
	radians *= dt;
	MATRIX4X4 rotate = {
		1, 0, 0, 0,
		0, cos(radians), (-sin(radians)), 0,
		0, sin(radians), cos(radians), 0,
		0, 0, 0, 1
	};

	return Mult_4x4(rotate, A);
}

MATRIX4X4 RotateY_Local(MATRIX4X4 A, float dt){
	float radians = 500.0f;
	radians *= dt;
	MATRIX4X4 rotate = {
		cos(radians), 0, sin(radians), 0,
		0, 1, 0, 0,
		-sin(radians), 0, cos(radians), 0,
		0, 0, 0, 1 };

	return Mult_4x4(rotate, A);
}

MATRIX4X4 RotateZ_Local(MATRIX4X4 A, float dt){
	float radians = 500.0f;
	radians *= dt;
	MATRIX4X4 rotate = {
		cos(radians), 0, sin(radians), 0,
		0, 1, 0, 0,
		-sin(radians), 0, cos(radians), 0,
		0, 0, 0, 1 };

	return Mult_4x4(rotate, A);
}

MATRIX4X4 Translate(MATRIX4X4 A, float x, float y, float z){
	MATRIX4X4 trans = {
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		x, y, z, 1 };

	return Mult_4x4(A, trans);
}

MATRIX3X3 Transpose(MATRIX4X4 A){
	MATRIX3X3 trans = {
		A.a, A.e, A.i,
		A.b, A.f, A.j,
		A.c, A.g, A.k
	};
	return trans;
}

MATRIX4X4 Transpose_4x4(MATRIX4X4 A){
	MATRIX4X4 trans = {
		A.a, A.e, A.i, A.m,
		A.b, A.f, A.j, A.n,
		A.c, A.g, A.k, A.o,
		A.d, A.h, A.l, A.p
	};
	return trans;
}

FLOAT4 Negate_Vec3(FLOAT4 A){
	FLOAT4 negate;
	negate.x = -A.x;
	negate.y = -A.y;
	negate.z = -A.z;
	negate.w = -A.w;
	return negate;
}

MATRIX4X4 FastInverse(MATRIX4X4 Mat){
	MATRIX3X3 transpose = Transpose(Mat);
	FLOAT4 position = { Mat.m, Mat.n, Mat.o, 1 };

	position = Mult_Vertex3x3(position, transpose);

	position = Negate_Vec3(position);

	Mat.m = position.x;
	Mat.n = position.y;
	Mat.o = position.z;

	return Mat;
}

MATRIX4X4 XMConverter(XMMATRIX& A){
	MATRIX4X4 output = {
		A.r[0].m128_f32[0], A.r[0].m128_f32[1], A.r[0].m128_f32[2], A.r[0].m128_f32[3],
		A.r[1].m128_f32[0], A.r[1].m128_f32[1], A.r[1].m128_f32[2], A.r[1].m128_f32[3],
		A.r[2].m128_f32[0], A.r[2].m128_f32[1], A.r[2].m128_f32[2], A.r[2].m128_f32[3],
		A.r[3].m128_f32[0], A.r[3].m128_f32[1], A.r[3].m128_f32[2], A.r[3].m128_f32[3]
	};
	return output;
}

DirectX::XMMATRIX XMConverter(MATRIX4X4 A){
	DirectX::XMMATRIX mat = {
		A.a, A.b, A.c, A.d,
		A.e, A.f, A.g, A.h,
		A.i, A.j, A.k, A.l,
		A.m, A.n, A.o, A.p
	};
	return mat;
}

MATRIX4X4 CreateViewMatrix(FLOAT4 EyePos, FLOAT4 FocusPos, FLOAT4 UpDir){

	 XMMATRIX tmp = XMMatrixLookAtLH(XMVectorSet(EyePos.x, EyePos.y, EyePos.z, EyePos.w),
		 XMVectorSet(FocusPos.x, FocusPos.y, FocusPos.z, FocusPos.w),
		 XMVectorSet(UpDir.x, UpDir.y, UpDir.z, UpDir.w));

	 MATRIX4X4 result = XMConverter(tmp);
	 return result;
}

MATRIX4X4 CreateProjectionMatrix(float zfar, float znear, unsigned int fov, float ar){
	MATRIX4X4 tmp;
	XMMATRIX proj = XMMatrixPerspectiveFovLH(XMConvertToRadians(fov), ar, znear, zfar);
	memcpy(&tmp, &proj, sizeof(float) * 16);
	return tmp;
}