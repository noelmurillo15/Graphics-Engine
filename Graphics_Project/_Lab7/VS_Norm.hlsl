#pragma pack_matrix(row_major)

Texture2D ObjTexture;
Texture2D ObjNormMap;
SamplerState ObjSamplerState;

cbuffer cbPerObject {
	float4x4 WVP;
	float4x4 World;
};

struct VS_OUTPUT {
	float4 Pos : SV_POSITION;
	float4 worldPos : TEXCOORD1;
	float2 tex : TEXCOORD;
	float3 Normal : COLOR;
	float3 tangent : TANGENT;
};


VS_OUTPUT main(float4 inPos : POSITION, float2 inTexCoord : TEXCOORD, float3 inNorm : COLOR, float3 tangent : TANGENT) {
	VS_OUTPUT output;

	output.Pos = mul(inPos, WVP);

	output.worldPos = mul(inPos, World);

	output.Normal = mul(inNorm, World);

	output.tangent = mul(tangent, World);

	output.tex = inTexCoord;

	return output;
}