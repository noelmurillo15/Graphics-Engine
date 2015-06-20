#pragma pack_matrix(row_major)

cbuffer cbPerObject {
	float4x4 WVP;
	float4x4 World;
};

struct INPUT_VERTEX {
	float4 pos : POSITION;
	float4 color : COLOR;
};

struct OUTPUT_VERTEX {
	float4 posOut : SV_POSITION;
	float4 colorOut : COLOR;
};


OUTPUT_VERTEX main(INPUT_VERTEX fromVertexBuffer)
{
	OUTPUT_VERTEX output = (OUTPUT_VERTEX)0;

	output.posOut = mul(fromVertexBuffer.pos, WVP);

	output.colorOut = fromVertexBuffer.color;

	return output;
}