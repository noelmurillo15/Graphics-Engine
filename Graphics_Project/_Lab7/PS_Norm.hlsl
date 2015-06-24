
Texture2D ObjTexture;
Texture2D ObjNormMap;
SamplerState ObjSamplerState;

struct VS_INPUT {
	float4 pos : SV_POSITION;
	float4 worldPos : TEXCOORD1;
	float2 tex : TEXCOORD0;
	float3 norm : COLOR;
	float3 tangent : TANGENT;
	float3 biTan : TEXCOORD2;
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


float4 main(VS_INPUT input) : SV_TARGET {

	float3 n = normalize(input.norm);
	//	Surface color
	float4 diffuse = ObjTexture.Sample(ObjSamplerState, input.tex);

	//	Normal Map
	float4 normalMap = ObjNormMap.Sample(ObjSamplerState, input.tex);
	normalMap = (2.0f * normalMap) - 1.0f;

	input.tangent = normalize(input.tangent - dot(input.tangent, n) * n);

	float3x3 texSpace = float3x3(input.tangent, input.biTan, n);
	input.norm = normalize(mul(normalMap, texSpace));
	
	//	Point Light
	float3 lightDir = normalize(light.position - input.worldPos);
	float lightRatio = clamp(dot(lightDir, n), 0, 1);
	float3 result = lightRatio * light.ambient * diffuse;

	//	Attenuation
	float attenuation = 1.0f - clamp((length(light.position - input.worldPos) / light.range), 0, 1);
	result *= attenuation;

	return float4(result, diffuse.a);
}