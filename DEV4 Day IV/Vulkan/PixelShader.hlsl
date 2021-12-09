// TODO: Part 2b
[[vk::push_constant]]
cbuffer MeshData
{
    uint mesh_ID;
    matrix pad;
    matrix pad2;
};

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

struct OutVertex
{
    float4 pos : SV_POSITION; // Homogenius Projection Space
    float2 uv;
    float3 nrm : NORMAL;
    float4 posW : WORLD;
};
// TODO: Part 4g
// TODO: Part 2i
StructuredBuffer<SHADER_MODEL_DATA> sceneData;

// TODO: Part 3e
// an ultra simple hlsl pixel shader
// TODO: Part 4b
float4 main(OutVertex outVert) : SV_TARGET
{
    float ratio = saturate(dot(normalize(-sceneData[0].sunDirection.xyz), normalize(outVert.nrm)));
	// TODO: Part 3a
    float4 color = float4(sceneData[0].materials[mesh_ID].Kd, 0) * sceneData[0].sunColor * ratio;
    //color = ratio * sceneData[0].sunColor * float4(sceneData[0].materials[mesh_ID].Kd, 0);
	// TODO: Part 4c
	// TODO: Part 4g (half-vector or reflect method your choice)
	return color; // TODO: Part 1a


}