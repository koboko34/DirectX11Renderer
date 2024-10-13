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
	
	float4 Diffuse = float4(0.f, 0.f, 0.f, 0.f);
    float4 Specular = float4(0.f, 0.f, 0.f, 0.f);
	if (DiffuseFactor > 0.f)
    {
        Diffuse = float4(DiffuseColor.rgb * DiffuseFactor, 1.f);
		
		float3 PixelToCam = normalize(CameraPos - p.WorldPos);
		float3 HalfwayVec = normalize(PixelToCam + PixelToLight);
		float SpecularFactor = pow(saturate(dot(p.Normal, HalfwayVec)), SpecularPower);
		Specular = float4(DiffuseColor.rgb * SpecularFactor, 1.f);		
    }
	
	float Attenuation = saturate(1.0 - (Distance * Distance) / (Radius * Radius)); // less control than constant, linear and quadratic, but guaranteed to reach 0 past max radius
	Diffuse *= Attenuation;
    Specular *= Attenuation;
	
    return saturate(Ambient + Diffuse + Specular);
}