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

float2 GetHeightmapUV(float3 Pos)
{
	float HalfPlaneDimension = PlaneDimension / 2.f;
	float x = Remap(Pos.x, -HalfPlaneDimension, HalfPlaneDimension, 0.f, 1.f);
	float z = Remap(Pos.z, -HalfPlaneDimension, HalfPlaneDimension, 0.f, 1.f);
	z = 1.f - z;
	
	return float2(x, z);
}

float hash(float2 p)
{
	return frac(sin(dot(p, float2(127.1, 311.7))) * 43758.5453);
}

float noise(float2 p)
{
	float2 i = floor(p);
	float2 f = frac(p);

	float a = hash(i);
	float b = hash(i + float2(1.0, 0.0));
	float c = hash(i + float2(0.0, 1.0));
	float d = hash(i + float2(1.0, 1.0));

	float2 u = f * f * (3.0 - 2.0 * f);

	return lerp(lerp(a, b, u.x), lerp(c, d, u.x), u.y);
}

uint GenerateChunkID(float2 v)
{
	int2 i = int2(v * 65536.0f);
	uint hash = uint(i.x) * 73856093u ^ uint(i.y) * 19349663u;
	hash ^= (hash >> 13);
	hash *= 0x85ebca6bu;
	hash ^= (hash >> 16);

	return hash;
}

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
	o.UV = GetHeightmapUV(o.WorldPos);
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