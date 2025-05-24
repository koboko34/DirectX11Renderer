#include "Common.hlsl"

struct PS_In
{
	float4 Pos : SV_POSITION;
	float3 WorldPos : WORLDPOS;
	float2 UV : TEXCOORD0;
	uint ChunkID : TEXCOORD1;
	float HeightAlongBlade : TEXCOORD2;
	uint LOD : TEXCOORD3;
};

static const float3 AOColor		= float3(0.0f,  0.1f, 0.0f);
static const float3 RootColor	= float3(0.0f,  0.3f, 0.0f);
static const float3 MidColor	= float3(0.4f,  0.7f, 0.1f);
static const float3 TipColor	= float3(0.9f,  1.0f, 0.2f);

float4 main(PS_In p) : SV_TARGET
{
	if (p.LOD == 1u)
	{
		return float4(1.f, 0.f, 0.f, 1.f);
	}
	
	float3 Color = RootColor;
	
	float TipThreshold = 2.f;
	float MidThreshold = 1.f;
	float RootThreshold = 0.2f;
	float AOThreshold = 0.f;
	
	if (p.HeightAlongBlade > MidThreshold)
	{
		Color = lerp(MidColor, TipColor, Remap(p.HeightAlongBlade, MidThreshold, TipThreshold, 0.f, 1.f));
	}
	else if (p.HeightAlongBlade > RootThreshold)
	{
		Color = lerp(RootColor, MidColor, Remap(p.HeightAlongBlade, RootThreshold, MidThreshold, 0.f, 1.f));
	}
	else if (p.HeightAlongBlade > AOThreshold)
	{
		Color = lerp(AOColor, RootColor, Remap(p.HeightAlongBlade, AOThreshold, RootThreshold, 0.f, 1.f));
	}
	
	return float4(Color, 1.f);
}