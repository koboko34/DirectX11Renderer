#include "Common.hlsl"

StructuredBuffer<float4> Corners : register(t0);

cbuffer CameraBuffer : register(b0)
{
	float4x4 ViewProj;
}

struct VS_IN
{
	float4 Pos : POSITION;
	uint VertexID : SV_VertexID;
	uint InstanceID : SV_InstanceID;
};

float4 main(VS_IN v) : SV_POSITION
{
	return mul(Corners[v.InstanceID * 8 + v.VertexID], ViewProj);
}