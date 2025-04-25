cbuffer ViewProjBuffer
{
	float4x4 ViewProj;
};

float4 main(float3 pos : POSITION) : SV_POSITION
{
	return mul(float4(pos, 1.f), ViewProj);
}