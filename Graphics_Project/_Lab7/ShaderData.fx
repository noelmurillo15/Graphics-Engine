#pragma pack_matrix(row_major)

cbuffer cbPerObject
{
	float4x4 WVP;
	float4x4 World;
};

struct Light{
	float3 dir;
	float pad;

	float3 position;
	float  range;
	float3 attenuation;
	float pad2;

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
	float4 worldPos : TEXCOORD1;
	float2 TexCoord : TEXCOORD;
	float3 Normal : COLOR;
};

VS_OUTPUT main(float4 inPos : POSITION, float2 inTexCoord : TEXCOORD, float3 inNorm : COLOR)
{
	VS_OUTPUT output;

	output.Pos = mul(inPos, WVP);

	output.worldPos = mul(inPos, World);

	output.Normal = mul(inNorm, World);

	output.TexCoord = inTexCoord;

	return output;
}

float4 PS(VS_OUTPUT input) : SV_TARGET
{

	 
	float3 colorFinal = float3(0.0f, 0.0f, 0.0f);	
	float distance, howMuchLight;

	float3 n = normalize(input.Normal);

		//	Attenuation
	float attenuation = 1.0f - clamp((length(light.position - input.worldPos) / light.range), 0, 1);


		//	Surface color
		float4 diffuse = ObjTexture.Sample(ObjSamplerState, input.TexCoord);
		//	Direction of light
		float3 lightDir = normalize(light.position - input.worldPos);
		//	Ratio of light
		float lightRatio = clamp(dot(lightDir, n), 0, 1);
		//	Mult em all up
		float3 result = lightRatio * light.ambient * diffuse;

		result *= attenuation;
		
	return float4(result, diffuse.a);
}

float4 PS_D2D(VS_OUTPUT input) : SV_TARGET
{

	float4 diffuse = ObjTexture.Sample(ObjSamplerState, input.TexCoord);

		return diffuse;
}