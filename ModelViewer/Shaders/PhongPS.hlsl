Texture2D diffuseTexture : register(t0);
Texture2D specularTexture : register(t1);
SamplerState samplerState : register(s0);

struct LightingData
{
	float3 CameraPos;
	float Radius;
	float3 LightPos;
	float SpecularPower;
	float3 LightColor;
	float Padding;
};

cbuffer Lighting : register(b0)
{
	LightingData Lighting;
};

struct MaterialData
{
	float3 DiffuseColor;
	int DiffuseSRV;
	float Specular;
	int SpecularSRV;
	float2 Padding;
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
	
	float AmbientFactor = 0.1f;
	float4 Ambient = float4(Color.rgb * AmbientFactor, 1.f);
	
	float Distance = distance(p.WorldPos, Lighting.LightPos);
	if (Distance > Lighting.Radius)
	{
		return Ambient;
	}
	
	float3 PixelToLight = normalize(Lighting.LightPos - p.WorldPos);
	float DiffuseFactor = saturate(dot(PixelToLight, p.WorldNormal));	
	
	float4 Diffuse = float4(0.f, 0.f, 0.f, 0.f);
    float4 Specular = float4(0.f, 0.f, 0.f, 0.f);
	if (DiffuseFactor > 0.f)
    {
        Diffuse = float4(Lighting.LightColor.rgb * Color.rgb * DiffuseFactor, 1.f);
		
		float3 PixelToCam = normalize(Lighting.CameraPos - p.WorldPos);
		float3 HalfwayVec = normalize(PixelToCam + PixelToLight);
		float SpecularFactor = pow(saturate(dot(p.WorldNormal, HalfwayVec)), Lighting.SpecularPower);
		Specular = float4(Lighting.LightColor.rgb * SpecularFactor, 1.f);		
    }
	
	float Attenuation = saturate(1.0 - (Distance * Distance) / (Lighting.Radius * Lighting.Radius)); // less control than constant, linear and quadratic, but guaranteed to reach 0 past max radius
	Diffuse *= Attenuation;
    Specular *= Attenuation;
	
    return saturate(Ambient + Diffuse + Specular);
}