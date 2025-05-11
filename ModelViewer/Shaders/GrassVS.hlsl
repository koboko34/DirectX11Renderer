#include "Common.hlsl"

Texture2D Heightmap : register(t0);
StructuredBuffer<float4x4> ChunkTransforms : register(t1);
StructuredBuffer<float4x4> GrassOffsets : register(t2);

SamplerState Sampler : register(s0);

cbuffer PlaneInfoBuffer : register(b0)
{
	float PlaneDimension;
	float HeightDisplacement;
	uint ChunkInstanceCount;
	bool bVisualiseChunks;
	float4x4 ChunkScaleMatrix;
	uint GrassPerChunk;
	float Time;
	float2 Padding;
};

cbuffer CameraBuffer : register(b1)
{
	float4x4 ViewProj;
};

struct VS_In
{
	float3 Pos : POSITION;
	uint InstanceID : SV_InstanceID;
};

struct VS_Out
{
	float4 Pos : SV_POSITION;
	float3 WorldPos : WORLDPOS;
	float2 UV : TEXCOORD0;
	uint ChunkID : TEXCOORD1;
	float HeightAlongBlade : TEXCOORD2;
};

VS_Out main(VS_In v)
{
	VS_Out o;

	uint OffsetID = v.InstanceID % GrassPerChunk;
	uint ChunkID = v.InstanceID / GrassPerChunk;
	
	// offset grass within its own chunk
	o.WorldPos = mul(float4(v.Pos, 1.f), GrassOffsets[OffsetID]).xyz;
	
	// transform by chunk transform
	o.WorldPos = mul(float4(o.WorldPos, 1.f), ChunkTransforms[ChunkID]).xyz;
	
	// apply height offset
	o.UV = GetHeightmapUV(o.WorldPos, PlaneDimension);
	float Height = Heightmap.SampleLevel(Sampler, o.UV, 0.f).r * HeightDisplacement;
	o.WorldPos.y += Height;
	
	// apply wind
	float PhaseX = (float)OffsetID * 0.37f;
	float PhaseZ = (float) OffsetID * 0.52f;
	o.WorldPos.x += sin(Time * 0.9f + PhaseX) * v.Pos.y * v.Pos.y * 0.5f;
	o.WorldPos.z += sin(Time * 2.3f + PhaseZ) * v.Pos.y * v.Pos.y * 0.5f;
		
	o.Pos = mul(float4(o.WorldPos, 1.f), ViewProj);	
	o.ChunkID = GenerateChunkID(float2(o.Pos.x, o.Pos.z));
	o.HeightAlongBlade = v.Pos.y;
	
	return o;
}