#pragma pack_matrix(row_major)

#define NUM_LEAVES_PER_TREE 200

cbuffer cbPerTree {
	float4x4 WVP;
	float4x4 World;

	bool isLeaf;
	float3 padding;
};

cbuffer cbPerScene {
	float4x4 leafOnTree[NUM_LEAVES_PER_TREE];
};


struct VS_OUTPUT {
	float4 pos : SV_POSITION;
	float4 worldPos : TEXCOORD1;
	float2 tex : TEXCOORD;
	float3 normal : COLOR;
};


VS_OUTPUT main(float3 inPos : POSITION, float2 inTex : TEXCOORD, float3 inNorm : COLOR, float3 instancePos : INSTANCEPOS, uint instanceID : SV_InstanceID) {
	VS_OUTPUT output;

		// get leaves position on tree, then add trees position
		if (isLeaf)
		{
			// We have 500 leaves per tree, so we can find the current leaf (in the tree) we are on (so we can get it's matrix from the matrix array stored in cbPerScene)
			// by first getting the current tree (instanceID / NUM_LEAVES_PER_TREE). We can then find the current leaf in the tree we are on by multiplying the current tree id
			// with the number of leaves per tree, then subtracting that total from the current instance id.
			uint currTree = (instanceID / NUM_LEAVES_PER_TREE);
			uint currLeafInTree = instanceID - (currTree * NUM_LEAVES_PER_TREE);
			inPos = mul(inPos, leafOnTree[currLeafInTree]);
		}

		// set position using instance data
		inPos += float4(instancePos, 0.0f);


	output.pos = mul(float4(inPos, 1.0f), WVP);

	output.worldPos = mul(float4(inPos, 1.0f), World);

	output.normal = mul(inNorm, World);

	//output.tangent = mul(inTan, World);

	//float3 bi = cross(inNorm, inTan);
	//output.biTan = mul(bi, World);

	output.tex = inTex;

	return output;
}