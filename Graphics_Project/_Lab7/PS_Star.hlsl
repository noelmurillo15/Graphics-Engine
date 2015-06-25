
struct INPUT_VERTEX {
	float4 posOut : SV_POSITION;
	float4 colorOut : COLOR;
};


float4 main(INPUT_VERTEX input) : SV_TARGET{
	return input.colorOut;
}