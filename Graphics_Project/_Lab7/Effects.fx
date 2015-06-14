#pragma pack_matrix(row_major)

cbuffer cbPerObject
{
	float4x4 WVP;
};

Texture2D ObjTexture;
SamplerState ObjSamplerState;

struct VS_OUTPUT
{
	float4 Pos : SV_POSITION;
	float2 TexCoord : TEXCOORD;
	float3 Normal : COLOR;
};

VS_OUTPUT main(float4 inPos : POSITION, float2 inTexCoord : TEXCOORD, float3 inNorm : COLOR)
{
	VS_OUTPUT output;

	output.Pos = mul(inPos, WVP);
	output.TexCoord = inTexCoord;
	output.Normal = inNorm;

	return output;
}

float4 PS(VS_OUTPUT input) : SV_TARGET
{
	return ObjTexture.Sample(ObjSamplerState, input.TexCoord);
}