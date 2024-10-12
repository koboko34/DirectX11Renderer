cbuffer LightingBuffer
{
	float3 CameraPos;
	float Radius;
	float3 LightPos;
	float SpecularPower;
	float3 DiffuseColor;
	float Padding;
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
	float4 Color = float4(1.f, 1.f, 1.f, 1.f);
	
	float AmbientFactor = 0.1f;
	float4 Ambient = float4(Color.rgb * AmbientFactor, 1.f);
	
	float Distance = distance(p.WorldPos, LightPos);
	if (Distance > Radius)
	{
		return Ambient;
	}
	
	float3 PixelToLight = normalize(LightPos - p.WorldPos);
	float DiffuseFactor = saturate(dot(PixelToLight, p.Normal));	
	float4 Diffuse = float4(DiffuseColor.rgb * DiffuseFactor, 1.f);
	
	//float3 PixelToCam = normalize(CameraPos - p.WorldPos);
	
	float Attenuation = saturate(1.0 - (Distance * Distance) / (Radius * Radius));
	Diffuse *= Attenuation;
	
	return saturate(Ambient + Diffuse);
}