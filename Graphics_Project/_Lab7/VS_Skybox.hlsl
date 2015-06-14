#pragma pack_matrix(row_major)

struct INPUT_CUBE_VERTEX {
	float3 coordinate : POSITION;
	float3 uvs : TEXCOORD;
};

struct OUTPUT_CUBE_VERTEX {
	float4 posOut : SV_POSITION;
	float2 uvOut : TEXCOORD;
};

cbuffer cbPerObject
{
	float4x4 WVP;
};


OUTPUT_CUBE_VERTEX main(INPUT_CUBE_VERTEX fromVertexBuffer) {
	OUTPUT_CUBE_VERTEX sendToRasterizer = (OUTPUT_CUBE_VERTEX)0;
	sendToRasterizer.posOut = float4(fromVertexBuffer.coordinate, 1);	//	converts f3 to f4 and makes w = 1

	sendToRasterizer.posOut = mul(sendToRasterizer.posOut, WVP);

	sendToRasterizer.uvOut = float2(fromVertexBuffer.uvs.xy);	//	uvs
	return sendToRasterizer;
}