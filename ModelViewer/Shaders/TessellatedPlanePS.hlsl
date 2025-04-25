Texture2D Heightmap : register(t0);
SamplerState Sampler : register(s0);

cbuffer PlaneInfoBuffer : register(b1)
{
	float PlaneDimension;
	float HeightDisplacement;
	float Padding;
	bool bVisualiseChunks;
};

struct PS_In
{
	float4 Pos : SV_POSITION;
	float2 UV : TEXCOORD0;
	uint ChunkID : TEXCOORD1;
};

float3 RandomRGB(uint seed)
{
    // Jenkins-style hash
	seed = (seed ^ 61) ^ (seed >> 16);
	seed *= 9;
	seed = seed ^ (seed >> 4);
	seed *= 0x27d4eb2d;
	seed = seed ^ (seed >> 15);

    // Generate three pseudo-random floats between 0.0 and 1.0
	float r = ((seed & 0xFF)) / 255.0;
	float g = ((seed & 0xFF00)) / 65280.0;
	float b = ((seed & 0xFF0000) >> 16) / 255.0;

	return float3(r, g, b);
}

float4 main(PS_In p) : SV_TARGET
{
	if (bVisualiseChunks)
		return float4(RandomRGB(p.ChunkID), 1.f);

	return float4(Heightmap.Sample(Sampler, p.UV).rrr, 1.f);
}