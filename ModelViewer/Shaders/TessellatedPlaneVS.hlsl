cbuffer TransformBuffer
{
	float4x4 Transform;
};

float3 main( float3 Pos : POSITION ) : POSITION
{
	return mul(float4(Pos, 1.f), Transform).xyz;
}