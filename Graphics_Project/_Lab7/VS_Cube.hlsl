#pragma pack_matrix(row_major)

struct INPUT_CUBE_VERTEX {
	float3 coordinate : POSITION;
	float3 uvs : TEXCOORD;
};

struct OUTPUT_CUBE_VERTEX {
	float4 projectedCoordinate : SV_POSITION;
	float2 uvOut : TEXCOORD;
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


OUTPUT_CUBE_VERTEX main(INPUT_CUBE_VERTEX fromVertexBuffer) {
	OUTPUT_CUBE_VERTEX sendToRasterizer = (OUTPUT_CUBE_VERTEX)0;
	sendToRasterizer.projectedCoordinate = float4(fromVertexBuffer.coordinate, 1);	//	converts f3 to f4 and makes w = 1

	sendToRasterizer.projectedCoordinate = mul(sendToRasterizer.projectedCoordinate, cube);
	//sendToRasterizer.projectedCoordinate = mul(sendToRasterizer.projectedCoordinate, view);
	sendToRasterizer.projectedCoordinate = mul(sendToRasterizer.projectedCoordinate, projection);

	sendToRasterizer.uvOut = float2(fromVertexBuffer.uvs.xy);	//	uvs
	return sendToRasterizer;
}