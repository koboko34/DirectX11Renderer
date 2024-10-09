struct PS_In
{
	float4 Pos : SV_POSITION;
	float2 TexCoord : TEXCOORD0;
	float4 Normal : NORMAL;
};

float4 main(PS_In p) : SV_TARGET
{
	return float4(p.Normal.xyz, 1.f);
}