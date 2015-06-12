texture2D diffuse : register(t0);
SamplerState filter : register(s0);


struct INPUT_PIXEL_CUBE {
	float4 projectedCoordinate : SV_POSITION;
	float2 uvOut : TEXCOORD;
};

float4 main(INPUT_PIXEL_CUBE toScreen) : SV_TARGET{
	float4 bgra = diffuse.Sample(filter, toScreen.uvOut);
	float4 argb;

	argb.b = bgra.a;
	argb.g = bgra.r;
	argb.r = bgra.g;
	argb.a = bgra.b;

	return argb;
}