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
// an ultra simple hlsl pixel shader
// TODO: Part 4b
float4 main() : SV_TARGET
{
	return float4(1.0f ,1.0f, 1.0f, 0); // TODO: Part 1a
	// TODO: Part 3a
	// TODO: Part 4c
	// TODO: Part 4g (half-vector or reflect method your choice)
}