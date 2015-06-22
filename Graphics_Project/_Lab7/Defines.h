#ifndef _DEFINES_H_
#define _DEFINES_H_

#include <vector>
#include <d3d11.h>
#pragma comment (lib, "d3d11.lib")
using namespace std;


struct FLOAT2{
	float u, v;

	FLOAT2() = default;
	FLOAT2(float _u, float _v){
		u = _u; v = _v;
	}
};

struct FLOAT3{
	float x, y, z;

	FLOAT3() = default;
	FLOAT3(float _x, float _y, float _z){
		x = _x; y = _y; z = _z;
	}
};

struct FLOAT4{
	float x, y, z, w;

	FLOAT4() = default;
	FLOAT4(float _x, float _y, float _z, float _w){
		x = _x; y = _y; z = _z; w = _w;
	}
};

struct COLORGBA{
	float r, g, b, a;

	COLORGBA() = default;
	COLORGBA(float _r, float _g, float _b, float _a){
		r = _r; g = _g; b = _b; a = _a;
	}
};

struct SIMPLE_VERTEX{
	FLOAT4 pos;
	COLORGBA color;
};

struct VERTEX{

	VERTEX(){}
	VERTEX(float x, float y, float z,
		float u, float v,
		float nx, float ny, float nz) 
	: pos(x, y, z), texCoord(u, v), normal(nx, ny, nz){}

	FLOAT3 pos;
	FLOAT2 texCoord;
	FLOAT3 normal;

	FLOAT3 tangent;
	FLOAT3 biTangent;
};

struct Vert{
	FLOAT3 Pos;
	FLOAT2 Uvs;
	FLOAT3 Norms;
};

struct Model{
	std::vector<Vert> interleaved;
	std::vector<unsigned int> out_Indicies;
};


struct MATRIX3X3{
	float a, b, c;
	float i, j, k;
	float x, y, z;

	MATRIX3X3() = default;
	MATRIX3X3(float _a, float _b, float _c, float _i, float _j, float _k, float _x, float _y, float _z){
		a = _a; b = _b; c = _c;
		i = _i; j = _j; k = _k;
		x = _x; y = _y; z = _z;
	}
};

struct MATRIX4X4{
	float a, b, c, d;
	float e, f, g, h;
	float i, j, k, l;
	float m, n, o, p = 1;

	MATRIX4X4() = default;
	MATRIX4X4(float _a, float _b, float _c, float _d, float _e, float _f, float _g, float _h, float _i, float _j, float _k, float _l, float _m, float _n, float _o, float _p){
		a = _a; b = _b; c = _c; d = _d;
		e = _e; f = _f; g = _g; h = _h;
		i = _i; j = _j; k = _k; l = _l;
		m = _m; n = _n; o = _o; p = _p;
	}
};

struct Light{
	Light(){ ZeroMemory(this, sizeof(Light)); }

	FLOAT3 direction;
	float pad;

	FLOAT3 position;
	float range;

	FLOAT4 ambientColor;
};

	//	Constant buffer structs
struct cbPerObject{
	MATRIX4X4 WVP;
	MATRIX4X4 World;

	//BOOL hasNormMap;
};

struct cbPerFrame{
	Light light;
};

#endif