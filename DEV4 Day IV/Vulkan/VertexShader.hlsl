// TODO: 2i
#pragma pack_matrix(row_major)
[[vk::push_constant]]
cbuffer MeshData
{
    uint mesh_ID;
};
// an ultra simple hlsl vertex shader
// TODO: Part 2b
struct OBJ_ATTRIBUTES
{
	float3 Kd;
	float d;
	float3 Ks;
	float Ns;
	float3 Ka;
	float sharpness;
	float3 Tf;
	float Ni;
	float3 Ke;
	uint illum;
};

struct SHADER_MODEL_DATA
{
	float4 sunDirection;
	float4 sunColor; //lighting
	matrix view;
	matrix projection; //viewing
    //matrix world;

	matrix matrices[1024]; //world space transformations
	OBJ_ATTRIBUTES materials[1024]; //Color/texture of surface
};
// TODO: Part 4g
// TODO: Part 2i
StructuredBuffer<SHADER_MODEL_DATA> sceneData;

// TODO: Part 3e

// TODO: Part 4a
// TODO: Part 1f
struct Vertex
{
	float3 pos : POSITION;
    float3 uvw : XCOORD;
    float3 nrm : NORMAL;
};

struct OutVertex
{
	float4 pos : SV_POSITION; // Homogenius Projection Space
    float2 uv : XCOORD;
    float3 nrm : NORMAL;
    float4 posW : WORLD;
};


// TODO: Part 4b
OutVertex main(Vertex inputVertex)
{
	OutVertex output;
	// TODO: Part 1h
	// TODO: Part 2i
	output.pos = mul(float4(inputVertex.pos, 1), sceneData[0].matrices[mesh_ID]);
    output.posW = output.pos;
    output.uv = float2(inputVertex.uvw.xy);
	output.nrm = mul(float4(inputVertex.nrm, 0), sceneData[0].matrices[mesh_ID]);
	output.pos = mul(output.pos, sceneData[0].view);
	output.pos = mul(output.pos, sceneData[0].projection);
		// TODO: Part 4e
	// TODO: Part 4b
		// TODO: Part 4e
	return output;
}