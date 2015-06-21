#pragma pack_matrix(row_major)


cbuffer cbPerObject{
	float4x4 WVP;
	float4x4 World;
};

struct SKYBOX_OUTPUT{
	float4 pos : SV_POSITION;
	float3 tex : TEXCOORD;
};

SKYBOX_OUTPUT main(float3 inPos : POSITION, float2 inTex : TEXCOORD/*, float3 inNorm : COLOR*/){
	SKYBOX_OUTPUT output = (SKYBOX_OUTPUT)0;

	output.pos = mul(float4(inPos, 1.0f), WVP);

	output.tex = inPos;

	return output;
}