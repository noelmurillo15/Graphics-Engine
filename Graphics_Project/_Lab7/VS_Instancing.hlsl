#pragma pack_matrix(row_major)

cbuffer cbPerTree {
	float4x4 WVP;
	float4x4 World;

	bool isLeaf;
	float3 padding;
};

struct VS_OUTPUT {
	float4 pos : SV_POSITION;
	float4 worldPos : TEXCOORD1;
	float2 tex : TEXCOORD;
	float3 normal : COLOR;
};


VS_OUTPUT main(float3 inPos : POSITION, float2 inTex : TEXCOORD, float3 inNorm : COLOR, float3 instancePos : INSTANCEPOS) {
	VS_OUTPUT output;

	// set position using instance data
	inPos += float4(instancePos, 0.0f);

	output.pos = mul(float4(inPos, 1.0f), WVP);

	output.worldPos = mul(float4(inPos, 1.0f), World);

	output.normal = mul(inNorm, World);

	output.tex = inTex;

	return output;
}