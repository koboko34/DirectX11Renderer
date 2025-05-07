StructuredBuffer<float4x4> Transforms : register(t0);

AppendStructuredBuffer<float4x4> CulledTransforms : register(u0);
RWStructuredBuffer<uint> InstanceCount : register(u1);
RWByteAddressBuffer ArgsBuffer : register(u2);

cbuffer CullData : register(b0)
{
	float4 Corners[8];
	float4x4 ScaleMatrix;
	float4x4 ViewProj;
	uint SentInstanceCount;
	uint3 ThreadGroupCounts;
}

cbuffer InstanceCountMultiplierBuffer : register(b1)
{
	uint InstanceCountMultiplier;
	float3 Padding;
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

	for (int i = 0; i < 8; i++)
	{
		const float4x4 t = Transforms[FlattenedID];
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

