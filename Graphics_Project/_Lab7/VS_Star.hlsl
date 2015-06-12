#pragma pack_matrix(row_major)


struct INPUT_VERTEX {
	float4 color : COLOR;
	float4 coordinate : POSITION;
};

struct OUTPUT_VERTEX {
	float4 colorOut : COLOR;
	float4 projectedCoordinate : SV_POSITION;
};

cbuffer OBJECT : register(b1) {
	float4x4 world;
	float4x4 view;
	float4x4 projection;

	float4x4 cube;
	float4x4 grid;
	float4x4 star;
	float4x4 ground;
};

OUTPUT_VERTEX main(INPUT_VERTEX fromVertexBuffer)
{
	OUTPUT_VERTEX sendToRasterizer = (OUTPUT_VERTEX)0;
	sendToRasterizer.projectedCoordinate = fromVertexBuffer.coordinate;

	sendToRasterizer.projectedCoordinate = mul(fromVertexBuffer.coordinate, star);
	sendToRasterizer.projectedCoordinate = mul(sendToRasterizer.projectedCoordinate, view);
	sendToRasterizer.projectedCoordinate = mul(sendToRasterizer.projectedCoordinate, projection);

	sendToRasterizer.colorOut = fromVertexBuffer.color;

	return sendToRasterizer;
}