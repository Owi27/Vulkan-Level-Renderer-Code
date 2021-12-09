[[vk::push_constant]]
cbuffer SHADER_VARS
{
	matrix wm; //64 bytes
	matrix view;
};

// TODO: Part 2f, Part 3b
// TODO: Part 1c
struct Vertex
{
	float4 pos : POSITION;
};

float4 main(Vertex inputVertex)
{
	// TODO: Part 2d
	inputVertex.pos = mul(inputVertex.pos, wm);
	inputVertex.pos = mul(view, inputVertex.pos);
	// TODO: Part 2f, Part 3b
	return inputVertex.pos;
}