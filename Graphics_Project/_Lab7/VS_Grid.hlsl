#pragma pack_matrix(row_major)

struct INPUT_GRID_VERTEX {
	float4 coordinate : POSITION;
	float4 color : COLOR;
};

struct OUTPUT_GRID_VERTEX {
	float4 projectedCoordinate : SV_POSITION;
	float4 colorOut : COLOR;
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


OUTPUT_GRID_VERTEX main(INPUT_GRID_VERTEX fromVertexBuffer) {
	OUTPUT_GRID_VERTEX sendToRasterizer = (OUTPUT_GRID_VERTEX)0;
	sendToRasterizer.projectedCoordinate = fromVertexBuffer.coordinate;

	sendToRasterizer.projectedCoordinate = mul(fromVertexBuffer.coordinate, grid);
	sendToRasterizer.projectedCoordinate = mul(sendToRasterizer.projectedCoordinate, view);
	sendToRasterizer.projectedCoordinate = mul(sendToRasterizer.projectedCoordinate, projection);

	sendToRasterizer.colorOut = fromVertexBuffer.color;

	return sendToRasterizer;
}