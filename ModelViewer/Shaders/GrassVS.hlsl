#include "Common.hlsl"

Texture2D Heightmap : register(t0);
StructuredBuffer<GrassData> Grass : register(t1);

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
	float2 GrassPos = Grass[v.InstanceID].Offset;
	
	o.ChunkID = Grass[v.InstanceID].ChunkID;
	o.HeightAlongBlade = v.Pos.y;
	
	// apply random rotation
	float3 RotatedPos = Rotate(float3(v.Pos.xy, 0.f), float3(0.f, 1.f, 0.f), RandomAngle(GrassPos));
	
	// apply height offset
	o.UV = GetHeightmapUV(GrassPos, PlaneDimension);
	float Height = Heightmap.SampleLevel(Sampler, o.UV, 0.f).r * HeightDisplacement;
	
	o.WorldPos = RotatedPos + float3(v.Pos, 0.f) + float3(GrassPos.x, Height, GrassPos.y);
	
	// apply wind if not root vertex
	if (v.Pos.y != 0.f)
	{		
		float2 Noise = PerlinNoise2D(GrassPos * 0.1f) * 8.f;
		float Input = dot(GrassPos + Noise, WindDir) - Time * TimeScale;
		float2 WindOffset = SumOfSines(Input, WindDir, FreqMultiplier, AmpMultiplier, WaveCount, Hash((float) o.ChunkID));
		WindOffset += PerlinNoise2D(GrassPos * Freq) * Amp;
		WindOffset *= pow(v.Pos.y, SwayExponent) * WindDir * WindStrength;
		o.WorldPos.xz += WindOffset;
	
		float WindAmount = length(WindOffset);
		float BendFactor = 0.3f;
		o.WorldPos.y -= min(WindAmount * BendFactor, 0.8f);
	}
	
	o.Pos = mul(float4(o.WorldPos, 1.f), ViewProj);
	
	return o;
}
