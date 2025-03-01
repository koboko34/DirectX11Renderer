Texture2D diffuseTexture : register(t0);
SamplerState samplerState : register(s0);

cbuffer Material
{
	float3 DiffuseColor;
	int DiffuseSRV;
};

struct PS_In
{
	float4 Pos : SV_POSITION;
	float3 WorldPos : POSITION;
	float2 TexCoord : TEXCOORD0;
	float3 Normal : NORMAL;
};

float4 main(PS_In p) : SV_TARGET
{
	if (DiffuseSRV >= 0)
	{
		return diffuseTexture.Sample(samplerState, p.TexCoord);
	}
	return float4(DiffuseColor, 1.f);
}