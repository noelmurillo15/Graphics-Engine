
Texture2D ObjTexture;
Texture2D ObjNormMap;
SamplerState ObjSamplerState;

struct VS_INPUT {
	float4 Pos : SV_POSITION;
	float4 worldPos : tex1;
	float2 tex : tex;
	float3 Normal : COLOR;
	float3 tangent : TANGENT;
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

	//	Surface color
	float4 diffuse = ObjTexture.Sample(ObjSamplerState, input.tex);

	//	Normal Mapping
	float4 normalMap = ObjNormMap.Sample(ObjSamplerState, input.tex);
	normalMap = (2.0f * normalMap) - 1.0f;
	input.tangent = normalize(input.tangent - dot(input.tangent, input.Normal) * input.Normal);

	float3 biTan = cross(input.Normal, input.tangent);
	float3x3 texSpace = float3x3(input.tangent, biTan, input.Normal);
	input.Normal = normalize(mul(normalMap, texSpace));

	float3 n = normalize(input.Normal);

	//	Point Light
	float3 lightDir = normalize(light.position - input.worldPos);
	float lightRatio = clamp(dot(lightDir, n), 0, 1);
	float3 result = lightRatio * light.ambient * diffuse;

	//	Attenuation
	float attenuation = 1.0f - clamp((length(light.position - input.worldPos) / light.range), 0, 1);
	result *= attenuation;

	return float4(result, diffuse.a);
}