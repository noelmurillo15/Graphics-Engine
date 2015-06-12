
struct INPUT_PIXEL_GRID {
	float4 projectedCoordinate : SV_POSITION;
	float4 colorOut : COLOR;
};


float4 main(INPUT_PIXEL_GRID toScreen) : SV_TARGET{
	return toScreen.colorOut;
}