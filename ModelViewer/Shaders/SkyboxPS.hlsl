TextureCube Tex : register(t0);
SamplerState Sampler : register(s0);

struct PSIn
{
	float3 WorldPos : TEXCOORD0;
	float4 Pos : SV_Position;
};

float4 main(PSIn i) : SV_TARGET
{
	return Tex.Sample(Sampler, normalize(i.WorldPos));
}