#pragma pack_matrix(row_major)

cbuffer cbPerObject
{
	float4x4 WVP;
	float4x4 World;
};

struct Light{
	float3 dir;
	float4 ambient;
	float4 diffuse;
};

cbuffer cbPerFrame{
	Light light;
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
	
	float4 localN = float4(inNorm.xyz, 0);
	localN = mul(localN, World);
	output.Normal = localN.xyz;

	//output.Normal = inNorm;

	return output;
}

float4 PS(VS_OUTPUT input) : SV_TARGET
{
	input.Normal = normalize(input.Normal);

	float4 diffuse = ObjTexture.Sample(ObjSamplerState, input.TexCoord);

	float3 finalColor;

	finalColor = diffuse * light.ambient;
	finalColor += saturate(dot(-light.dir, input.Normal) * light.diffuse * diffuse);

	return float4(finalColor, diffuse.a);
}

float4 PS_D2D(VS_OUTPUT input) : SV_TARGET
{
	float4 diffuse = ObjTexture.Sample(ObjSamplerState, input.TexCoord);

	return diffuse;
}