cbuffer LightingBuffer
{
	float3 CameraPos;
	float Padding;
	float3 LightPos;
	float SpecularPower;
};

struct PS_In
{
	float4 Pos : SV_POSITION;
	float4 WorldPos : POSITION;
	float2 TexCoord : TEXCOORD0;
	float3 Normal : NORMAL;
};

float4 main(PS_In p) : SV_TARGET
{
	float4 Color = float4(1.f, 1.f, 1.f, 1.f);
	
	float AmbientFactor = 0.1f;
	float4 Ambient = float4(Color.rgb * AmbientFactor, 1.f);
	
	//float3 PixelToCam = normalize(CameraPos - p.WorldPos.xyz);
	float3 PixelToLight = normalize(LightPos - p.WorldPos.xyz);
	
	float DiffuseFactor = saturate(dot(PixelToLight, p.Normal));	
	float4 Diffuse = float4(Color.rgb * DiffuseFactor, 1.f);
	
	return saturate(Ambient + Diffuse);
}