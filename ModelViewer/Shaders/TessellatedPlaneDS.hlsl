struct DS_Out
{
	float4 vPosition : SV_POSITION;
};

struct DS_In
{
	float3 vPosition : POSITION; 
};

cbuffer ViewProjMatrix
{
	float4x4 ViewProj;
};

struct TessFactors
{
	float EdgeTessFactor[4] : SV_TessFactor;
	float InsideTessFactor[2] : SV_InsideTessFactor;
};

#define NUM_CONTROL_POINTS 4

[domain("quad")]
DS_Out main(
	TessFactors t,
	float2 UV : SV_DomainLocation,
	const OutputPatch<DS_In, NUM_CONTROL_POINTS> Patch)
{
	DS_Out Output;

	float3 p0 = Patch[0].vPosition;
	float3 p1 = Patch[1].vPosition;
	float3 p2 = Patch[2].vPosition;
	float3 p3 = Patch[3].vPosition;
	
	float3 Top = lerp(p0, p1, UV.x);
	float3 Bot = lerp(p2, p3, UV.x);
	float3 Pos = lerp(Top, Bot, UV.y);
	
	Output.vPosition = mul(float4(Pos, 1.f), ViewProj);

	return Output;
}
