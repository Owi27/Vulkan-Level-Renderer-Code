//float4 main( float4 pos : POSITION ) : SV_POSITION
//{
//	return pos;
//}


// TODO: Part 4c (in new shader)
struct Vertex
{
	float2 pos : POSITION;
	float4 clr : COLOR;
};

// TODO: Part 4d (in new shader)
struct VERTEX_OUT
{
	float4 pos : SV_POSITION; //SV = System Value
	float4 clr : COLOR;
};

// TODO: Part 4e

// Vertex Shader
VERTEX_OUT main(Vertex _vertex)
{
	VERTEX_OUT tempOUT;
	tempOUT.pos = float4(_vertex.pos[0],_vertex.pos[1], 0, 1);
	tempOUT.clr = float4(_vertex.clr);
	return tempOUT;
}
