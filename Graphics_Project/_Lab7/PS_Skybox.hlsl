
TextureCube SkyMap;
SamplerState ObjSamplerState;

struct SKYBOX_INPUT{
	float4 pos : SV_POSITION;
	float3 tex : TEXCOORD;
};

float4 main(SKYBOX_INPUT input) : SV_Target{
	return SkyMap.Sample(ObjSamplerState, input.tex);
}