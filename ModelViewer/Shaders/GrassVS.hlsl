#include "Common.hlsl"

Texture2D Heightmap : register(t0);
StructuredBuffer<float2> ChunkOffsets : register(t1);
StructuredBuffer<float2> GrassOffsets : register(t2);

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

cbuffer WindBuffer : register(b2)
{
	float Freq;
	float Amp;
	float2 WindDir;
	float TimeScale;
	float FreqMultiplier;
	float AmpMultiplier;
	uint WaveCount;
	float WindStrength;
	float SwayExponent;
	float2 MorePadding;
}

struct VS_In
{
	float2 Pos : POSITION;
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
	
	// apply random rotation
	
	// offset grass within its own chunk
	o.WorldPos = float3(v.Pos, 0.f) + float3(GrassOffsets[OffsetID].x, 0.f, GrassOffsets[OffsetID].y);
	
	// offset by chunk position
	o.WorldPos += float3(ChunkOffsets[ChunkID].x, 0.f, ChunkOffsets[ChunkID].y);
	
	// apply height offset
	o.UV = GetHeightmapUV(o.WorldPos, PlaneDimension);
	float Height = Heightmap.SampleLevel(Sampler, o.UV, 0.f).r * HeightDisplacement;
	o.WorldPos.y += Height;
	
	// apply wind if not root vertex
	if (v.Pos.y != 0.f)
	{
		float4 GrassPos = float4(GrassOffsets[OffsetID].x + ChunkOffsets[ChunkID].x, 0.f, GrassOffsets[OffsetID].y + ChunkOffsets[ChunkID].y, 1.f);
		uint ChunkIDFromWorld = GenerateChunkID(ChunkOffsets[ChunkID]);
		float2 Noise = PerlinNoise2D(GrassPos.xz * 0.1f) * 8.f;
		float Input = dot(GrassPos.xz + Noise, WindDir) - Time * TimeScale;
		float2 WindOffset = SumOfSines(Input, WindDir, FreqMultiplier, AmpMultiplier, WaveCount, Hash((float)ChunkIDFromWorld));
		WindOffset += PerlinNoise2D(GrassPos.xz * Freq) * Amp;
		WindOffset *= pow(v.Pos.y, SwayExponent) * WindDir * WindStrength;
		o.WorldPos.xz += WindOffset;
	
		float WindAmount = length(WindOffset);
		float BendFactor = 0.3f;
		o.WorldPos.y -= min(WindAmount * BendFactor, 0.8f);
	}
	
	o.Pos = mul(float4(o.WorldPos, 1.f), ViewProj);	
	o.ChunkID = GenerateChunkID(float2(o.Pos.x, o.Pos.z));
	o.HeightAlongBlade = v.Pos.y;
	
	return o;
}
