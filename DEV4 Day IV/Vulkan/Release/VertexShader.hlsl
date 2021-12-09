// TODO: 2i
#pragma pack_matrix(row_major)
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
	float3 uvw;
	float3 nrm;
};

struct OutVertex
{
	float4 pos : SV_POSITION;
	float2 uv;
	float3 nrm;
};

// TODO: Part 4b
OutVertex main(Vertex inputVertex) : SV_POSITION
{
	OutVertex output;
	//output.pos = float4(inputVertex.pos, 1 );
	//output.uv = float2(inputVertex.uvw.x, inputVertex.uvw.y);
	//output.nrm = inputVertex.nrm;
	// TODO: Part 1h
	//output.pos.z += 0.75;
	//output.pos.y -= 0.75;
	// TODO: Part 2i
	output.pos = mul(float4(inputVertex.pos, 1), sceneData[0].matrices[0]);
	//output.nrm = mul(float4(inputVertex.norm, 0), sceneData[0].matrices[0]);
	output.pos = mul(output.pos, sceneData[0].view);
	output.pos = mul(output.pos, sceneData[0].projection);
		// TODO: Part 4e
	// TODO: Part 4b
		// TODO: Part 4e
	return output;
}