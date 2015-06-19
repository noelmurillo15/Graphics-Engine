Texture2D ObjTexture;
SamplerState ObjSamplerState;

struct VS_INPUT {
	float4 Pos : SV_POSITION;
	float4 worldPos : TEXCOORD1;
	float2 TexCoord : TEXCOORD;
	float3 Normal : COLOR;
};

struct Light{
	float3 dir;
	float pad;

	float3 position;
	float  range;

	float4 ambient;
};

cbuffer cbPerFrame{
	Light light;
};


float4 main(VS_INPUT input) : SV_TARGET{

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