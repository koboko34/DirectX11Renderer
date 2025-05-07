struct PS_In
{
	float4 Pos : SV_POSITION;
	float3 WorldPos : WORLDPOS;
	float2 UV : TEXCOORD0;
	uint ChunkID : TEXCOORD1;
};

float4 main(PS_In p) : SV_TARGET
{
	return float4(0.f, 0.3f, 0.f, 1.f);
}