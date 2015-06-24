#pragma pack_matrix(row_major)

cbuffer cbPerObject {
	float4x4 WVP;
	float4x4 World;
};

struct VS_OUTPUT {
	float4 pos : SV_POSITION;
	float4 worldPos : TEXCOORD1;
	float2 tex : TEXCOORD0;
	float3 norm : COLOR;
	float3 tangent : TANGENT;
	float3 biTan : TEXCOORD2;
};


VS_OUTPUT main(float3 inPos : POSITION, float2 inTexCoord : TEXCOORD, float3 inNorm : COLOR, float3 inTan : TANGENT) {
	VS_OUTPUT output = (VS_OUTPUT)0;

	output.pos = mul(float4(inPos, 1.0f), WVP);

	output.worldPos = mul(float4(inPos, 1.0f), World);

	output.norm = mul(inNorm, World);

	output.tangent = mul(inTan, World);

	float3 bi = cross(inNorm, inTan);
	output.biTan = mul(bi, World);
	
	output.tex = inTexCoord;

	return output;
}