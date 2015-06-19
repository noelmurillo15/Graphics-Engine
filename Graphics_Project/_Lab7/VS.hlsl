#pragma pack_matrix(row_major)

Texture2D ObjTexture;
SamplerState ObjSamplerState;

cbuffer cbPerObject {
	float4x4 WVP;
	float4x4 World;
};

struct VS_OUTPUT {
	float4 Pos : SV_POSITION;
	float4 worldPos : TEXCOORD1;
	float2 TexCoord : TEXCOORD;
	float3 Normal : COLOR;
};


VS_OUTPUT main(float4 inPos : POSITION, float2 inTexCoord : TEXCOORD, float3 inNorm : COLOR) {
	VS_OUTPUT output;

	output.Pos = mul(inPos, WVP);

	output.worldPos = mul(inPos, World);

	output.Normal = mul(inNorm, World);

	output.TexCoord = inTexCoord;

	return output;
}