#define MAX_PLANE_CHUNKS 64

cbuffer InstanceBuffer
{
	float4x4 Transforms[MAX_PLANE_CHUNKS];
};

struct VS_In
{
	float3 Pos : POSITION;
	uint InstanceID : SV_InstanceID;
};

float3 main(VS_In v) : POSITION
{
	return mul(float4(v.Pos, 1.f), Transforms[v.InstanceID]).xyz;
}