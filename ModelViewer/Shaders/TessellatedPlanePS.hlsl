Texture2D Heightmap : register(t0);
SamplerState Sampler : register(s0);

struct PS_In
{
	float4 Pos : SV_POSITION;
	float2 UV : TEXCOORD0;
};

float4 main(PS_In p) : SV_TARGET
{
	return float4(Heightmap.Sample(Sampler, p.UV).rrr, 1.f);
	//return float4(1.0f, 1.0f, 1.0f, 1.0f);
}