Texture2D diffuseTexture : register(t0);
Texture2D specularTexture : register(t1);
SamplerState samplerState : register(s0);

#define MAX_POINT_LIGHTS 8

struct PointLight
{
	float Radius;
	float3 LightPos;
	float SpecularPower;
	float3 LightColor;
};

cbuffer Lighting : register(b0)
{
	PointLight PointLights[MAX_POINT_LIGHTS];
	float3 CameraPos;
	int PointLightCount;
};

struct MaterialData
{
	float3 DiffuseColor;
	int DiffuseSRV;
	float3 Specular;
	int SpecularSRV;
};

cbuffer Material : register(b1)
{
	MaterialData Mat;
};

struct PS_In
{
	float4 Pos : SV_POSITION;
	float3 WorldPos : POSITION;
	float2 TexCoord : TEXCOORD0;
	float3 WorldNormal : NORMAL;
};

float4 main(PS_In p) : SV_TARGET
{		
	float4 Color;
	if (Mat.DiffuseSRV >= 0)
	{
		Color = diffuseTexture.Sample(samplerState, p.TexCoord);
	}
	else
	{
		Color = float4(Mat.DiffuseColor, 1.f);
	}
	
	clip(Color.a < 0.1f ? -1.f : 1.f); // play around with this number
	
	float BaseAlpha = Color.a;
	float AmbientFactor = 0.05f;
	float4 Ambient = float4(Color * AmbientFactor);
	
	float3 PixelToCam = normalize(CameraPos - p.WorldPos);
	float4 LightTotal = float4(0.f, 0.f, 0.f, 0.f);
	for (int i = 0; i < PointLightCount; i++)
	{
		float Distance = distance(p.WorldPos, PointLights[i].LightPos);
		if (Distance > PointLights[i].Radius)
		{
			continue;
		}
	
		float3 PixelToLight = normalize(PointLights[i].LightPos - p.WorldPos);
		float DiffuseFactor = saturate(dot(PixelToLight, p.WorldNormal));	
	
		if (DiffuseFactor <= 0.f)
		{
			continue;
		}

		float4 Diffuse = float4(PointLights[i].LightColor, 1.f) * float4(Color.xyz, 0.5f) * DiffuseFactor;
		
		float3 HalfwayVec = normalize(PixelToCam + PixelToLight);
		float SpecularFactor = pow(saturate(dot(p.WorldNormal, HalfwayVec)), PointLights[i].SpecularPower);
		float4 Specular = float4(PointLights[i].LightColor, 1.f) * SpecularFactor;
	
		float Attenuation = saturate(1.0 - (Distance * Distance) / (PointLights[i].Radius * PointLights[i].Radius)); // less control than constant, linear and quadratic, but guaranteed to reach 0 past max radius
		LightTotal += Diffuse * Attenuation;
		LightTotal += Specular * Attenuation;
	}
	
	float4 outColor = saturate(Ambient + LightTotal);
    return outColor;
}