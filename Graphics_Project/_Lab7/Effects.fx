#pragma pack_matrix(row_major)


cbuffer OBJECT : register(b1) {
	float4x4 world;
	float4x4 view;
	float4x4 projection;

	float4x4 cube;
	float4x4 grid;
	float4x4 star;
	float4x4 ground;
};

Texture2D ObjTexture;
SamplerState ObjSamplerState;

struct VS_OUTPUT {
	float4 pos : SV_POSITION;
	float2 TexCoord : TEXCOORD;
};


VS_OUTPUT main(float3 inPos : POSITION, float2 inTexCoord : TEXCOORD)
{
	VS_OUTPUT output;

	float4 localH = float4(inPos, 1);

	output.pos = mul(localH, ground);
	output.pos = mul(output.pos, view);
	output.pos = mul(output.pos, projection);

	output.TexCoord = inTexCoord;

	return output;
}

float4 PS(VS_OUTPUT input) : SV_TARGET{
	return ObjTexture.Sample(ObjSamplerState, input.TexCoord);
}