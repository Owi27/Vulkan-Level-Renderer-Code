//Pixel Shader
struct VERTEX_OUT
{
	float4 pos : SV_POSITION; //SV = System Value
	float4 clr : COLOR;
};

float4 main(VERTEX_OUT _outVert) : SV_TARGET
{
	return float4(_outVert.clr);
//return float4(1.0f, 0.0f,0.0f, 1.0f); 
}
