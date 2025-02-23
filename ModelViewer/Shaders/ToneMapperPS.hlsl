Texture2D screenTexture : register(t0);
SamplerState samplerState : register(s0);

cbuffer ToneMapperBuffer
{
	float whiteLevel;
	float exposure;
	float bias;
	int formula; // 0 == Reinhard Basic, 1 == ReinhardExtended, 2 = ReinhardExtendedBias
};

#define REINHARD_BASIC 0
#define REINHARD_EXTENDED 1
#define REINHARD_EXTENDED_BIAS 2

struct PS_In
{
	float4 Pos : SV_POSITION;
	float2 TexCoord : TEXCOORD0;
	float3 Normal : NORMAL;
};

float3 ReinhardBasic(float3 color)
{
	return color / (1.f + color);
}

float3 ReinhardExtended(float3 color)
{
	float3 numerator = color * (1.f + color / (whiteLevel * whiteLevel));
	float3 denominator = 1.f + color;
	return numerator / denominator;
}

float3 ReinhardExtendedBias(float3 color)
{
	return color / (color + bias);
}

float4 main(PS_In p) : SV_TARGET
{
	float4 color = screenTexture.Sample(samplerState, p.TexCoord);
	float3 toneMappedColor = float3(0.f, 0.f, 0.f);
	
	if (formula == REINHARD_BASIC)
	{
		toneMappedColor = ReinhardBasic(color.xyz);
	}
	else if (formula == REINHARD_EXTENDED)
	{
		toneMappedColor = ReinhardExtended(color.xyz);
	}
	else if (formula == REINHARD_EXTENDED_BIAS)
	{
		toneMappedColor = ReinhardExtendedBias(color.xyz);
	}
	
	return float4(toneMappedColor, 1.f);
}
