#include "Common.hlsl"

SamplerState Sampler : register(s0);

StructuredBuffer<float4x4> Transforms : register(t0);
StructuredBuffer<float2> Offsets : register(t1);
StructuredBuffer<float2> CulledOffsets : register(t2);
Texture2D Heightmap : register(t3);

AppendStructuredBuffer<float4x4> CulledTransforms : register(u0);
AppendStructuredBuffer<float2> CulledOffsetsAppend : register(u1);
AppendStructuredBuffer<GrassData> CulledGrassData : register(u2);
RWStructuredBuffer<uint> InstanceCount : register(u3);
RWByteAddressBuffer ArgsBuffer : register(u4);

cbuffer CullData : register(b0)
{
	float4 Corners[8];
	float4x4 ScaleMatrix;
	float4x4 ViewProj;
	uint SentInstanceCount;
	uint3 ThreadGroupCounts;
	uint GrassPerChunk;
	uint PlaneDimension;
	float HeightDisplacement;
	float Padding;
}

cbuffer InstanceCountMultiplierBuffer : register(b1)
{
	uint InstanceCountMultiplier;
	float3 MorePadding;
}

static const uint tx = 32u;
static const uint ty = 1u;
static const uint tz = 1u;
[numthreads(tx, ty, tz)]
void FrustumCull( uint3 DTid : SV_DispatchThreadID )
{
	uint FlattenedID = DTid.z * ThreadGroupCounts.x * ThreadGroupCounts.y * tx * ty +
                       DTid.y * ThreadGroupCounts.x * tx +
                       DTid.x;
	
	if (FlattenedID >= SentInstanceCount)
		return;
	
	const float Bias = 0.01f;
	const float4x4 t = Transforms[FlattenedID];

	for (int i = 0; i < 8; i++)
	{
		float4 TransformedCorner = mul(mul(mul(Corners[i], ScaleMatrix), t), ViewProj);

		if (abs(TransformedCorner.x) <= TransformedCorner.w + Bias && abs(TransformedCorner.y) <= TransformedCorner.w + Bias &&
			(TransformedCorner.z >= -Bias && TransformedCorner.z <= TransformedCorner.w + Bias))
		{
			CulledTransforms.Append(t);
			InterlockedAdd(InstanceCount[0], 1u);
			return;
		}
	}
}

[numthreads(tx, ty, tz)]
void FrustumCullOffsets(uint3 DTid : SV_DispatchThreadID)
{
	uint FlattenedID = DTid.z * ThreadGroupCounts.x * ThreadGroupCounts.y * tx * ty +
                       DTid.y * ThreadGroupCounts.x * tx +
                       DTid.x;
	
	if (FlattenedID >= SentInstanceCount)
		return;
	
	const float Bias = 0.01f;
	const float4 o = float4(Offsets[FlattenedID].x, 0.f, Offsets[FlattenedID].y, 0.f);
	
	for (int i = 0; i < 8; i++)
	{
		float4 TransformedCorner = mul(mul(Corners[i], ScaleMatrix) + o, ViewProj);

		if (abs(TransformedCorner.x) <= TransformedCorner.w + Bias && abs(TransformedCorner.y) <= TransformedCorner.w + Bias &&
			(TransformedCorner.z >= -Bias && TransformedCorner.z <= TransformedCorner.w + Bias))
		{
			CulledOffsetsAppend.Append(o.xz);
			InterlockedAdd(InstanceCount[0], 1u);
			return;
		}
	}
}

static const uint grass_tx = 32u;
static const uint grass_ty = 8u;
static const uint grass_tz = 1u;

[numthreads(grass_tx, grass_ty, grass_tz)]
void FrustumCullGrass(uint3 DTid : SV_DispatchThreadID)
{
	uint GrassID = DTid.x;
	uint ChunkID = DTid.y;
	
	if (GrassID >= GrassPerChunk || ChunkID >= SentInstanceCount)
		return;
	
	const float Bias = 0.f; // this might be a bit too generous
	const float3 ChunkOffset = float3(CulledOffsets[ChunkID].x, 0.f, CulledOffsets[ChunkID].y);
	const float3 GrassOffset = float3(Offsets[GrassID].x, 0.f, Offsets[GrassID].y);
	const float4 WorldOffset = float4(ChunkOffset + GrassOffset, 0.f);
	
	const float2 UV = GetHeightmapUV(WorldOffset.xz, PlaneDimension);
	const float4 Height = float4(0.f, Heightmap.SampleLevel(Sampler, UV, 0.f).r * HeightDisplacement, 0.f, 0.f);
	
	for (int i = 0; i < 8; i++)
	{
		float4 TransformedCorner = mul(Corners[i] + WorldOffset + Height, ViewProj);

		if (abs(TransformedCorner.x) <= TransformedCorner.w + Bias && abs(TransformedCorner.y) <= TransformedCorner.w + Bias &&
			(TransformedCorner.z >= -Bias && TransformedCorner.z <= TransformedCorner.w + Bias))
		{
			GrassData Grass;
			Grass.Offset = WorldOffset.xz;
			Grass.ChunkID = HashFloat2ToUint(CulledOffsets[ChunkID]);
			Grass.Padding = 0.f;
			
			CulledGrassData.Append(Grass);
			InterlockedAdd(InstanceCount[0], 1u);
			return;
		}
	}
}

[numthreads(1, 1, 1)]
void ClearInstanceCount(uint3 DTid : SV_DispatchThreadID)
{
	InstanceCount[0] = 0u;
}

[numthreads(1, 1, 1)]
void TransferInstanceCount(uint3 DTid : SV_DispatchThreadID)
{
	ArgsBuffer.Store(4u, InstanceCount[0] * InstanceCountMultiplier);
}

