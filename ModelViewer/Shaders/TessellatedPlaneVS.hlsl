Texture2D Heightmap : register(t0);
SamplerState Sampler : register(s0);

#define MAX_PLANE_CHUNKS 64

cbuffer InstanceBuffer : register(b0)
{
	float4x4 Transforms[MAX_PLANE_CHUNKS];
};

cbuffer PlaneInfoBuffer : register(b1)
{
	float PlaneDimension;
	float HeightDisplacement;
	float Padding;
	bool bVisualiseChunks;
};

struct VS_In
{
	float3 Pos : POSITION;
	uint InstanceID : SV_InstanceID;
};

struct VS_Out
{
	float3 Pos : POSITION;
	float2 UV : TEXCOORD0;
	uint ChunkID : TEXCOORD1;
};

float Remap(float Value, float FromMin, float FromMax, float ToMin, float ToMax)
{
	return ToMin + (Value - FromMin) * (ToMax - ToMin) / (FromMax - FromMin);
}

float2 GetHeightmapUV(float3 Pos)
{
	float HalfPlaneDimension = PlaneDimension / 2.f;
	float x = Remap(Pos.x, -HalfPlaneDimension, HalfPlaneDimension, 0.f, 1.f);
	float z = Remap(Pos.z, -HalfPlaneDimension, HalfPlaneDimension, 0.f, 1.f);
	z = 1.f - z;
	
	return float2(x, z);
}

VS_Out main(VS_In v)
{
	VS_Out o;
	o.Pos = mul(float4(v.Pos, 1.f), Transforms[v.InstanceID]).xyz;
	o.UV = GetHeightmapUV(o.Pos);
	
	float Height = Heightmap.SampleLevel(Sampler, o.UV, 0.f).r * HeightDisplacement;
	o.Pos.y = Height;
	
	o.ChunkID = v.InstanceID;
	
	return o;
}