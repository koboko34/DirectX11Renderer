Texture2D screenTexture : register(t0);
SamplerState samplerState : register(s0);

struct PS_In
{
	float4 Pos : SV_POSITION;
	float3 Normal : NORMAL;
	float2 TexCoord : TEXCOORD0;
};

float4 main(PS_In p) : SV_TARGET
{
	return screenTexture.Sample(samplerState, p.TexCoord);
}