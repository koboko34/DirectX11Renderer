#include "Common.hlsl"

struct PS_In
{
	float4 Pos : SV_POSITION;
	float3 WorldPos : WORLDPOS;
	float2 UV : TEXCOORD0;
	uint ChunkID : TEXCOORD1;
	float HeightAlongBlade : TEXCOORD2;
};

static const float3 RootColor	= float3(0.0f,  0.3f, 0.0f);
static const float3 TipColor	= float3(0.4f,  0.7f, 0.1f);
static const float3 AOColor		= float3(0.0f,  0.1f, 0.0f);

float4 main(PS_In p) : SV_TARGET
{
	float3 Color = RootColor;
	
	float TipThreshold = 0.2f;
	float AOThreshold = 0.2f;
	
	if (p.HeightAlongBlade > TipThreshold)
	{
		Color = lerp(RootColor, TipColor, Remap(p.HeightAlongBlade, TipThreshold, 1.f, 0.f, 1.f));
	}
	else if (p.HeightAlongBlade < AOThreshold)
	{
		Color = lerp(AOColor, RootColor, Remap(p.HeightAlongBlade, 0.f, AOThreshold, 0.f, 1.f));
	}
	
	return float4(Color, 1.f);
}