struct HS_In
{
	float3 Pos : POSITION;
	float2 UV : TEXCOORD0;
};

struct HS_Out
{
	float3 Pos : POSITION;
	float2 UV : TEXCOORD0;
};

struct HS_CONSTANT_DATA_OUTPUT
{
	float EdgeTessFactor[4]			: SV_TessFactor;
	float InsideTessFactor[2]		: SV_InsideTessFactor;
};

cbuffer CameraBuffer
{
	float3 CameraPos;
	float TessellationScale;
};

#define NUM_CONTROL_POINTS 4

float Quantize(float x)
{
	return round(x);
}

float GetEdgeTessFactor(float3 v0, float3 v1)
{
	float3 AvgPos = (v0 + v1) / 2.f;
	float Dist = distance(AvgPos, CameraPos);
	
	float TessFactor = saturate(TessellationScale / Dist);
	TessFactor = lerp(2.f, 64.f, TessFactor);
	return Quantize(TessFactor);
}

HS_CONSTANT_DATA_OUTPUT CalcHSPatchConstants(
	InputPatch<HS_In, NUM_CONTROL_POINTS> ip,
	uint PatchID : SV_PrimitiveID)
{
	HS_CONSTANT_DATA_OUTPUT o;
	
	float Tess0 = GetEdgeTessFactor(ip[0].Pos, ip[1].Pos);
	float Tess1 = GetEdgeTessFactor(ip[1].Pos, ip[2].Pos);
	float Tess2 = GetEdgeTessFactor(ip[2].Pos, ip[3].Pos);
	float Tess3 = GetEdgeTessFactor(ip[3].Pos, ip[0].Pos);
	
	float InnerTess = max(max(Tess0, Tess1), max(Tess2, Tess3));

	o.EdgeTessFactor[0] = Tess0;
	o.EdgeTessFactor[1] = Tess1;
	o.EdgeTessFactor[2] = Tess2;
	o.EdgeTessFactor[3] = Tess3;
	
	o.InsideTessFactor[0] = InnerTess;
	o.InsideTessFactor[1] = InnerTess;

	return o;
}

[domain("quad")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(NUM_CONTROL_POINTS)]
[patchconstantfunc("CalcHSPatchConstants")]
HS_Out main( 
	InputPatch<HS_In, NUM_CONTROL_POINTS> ip, 
	uint i : SV_OutputControlPointID,
	uint PatchID : SV_PrimitiveID )
{
	HS_Out o;

	o.Pos = ip[i].Pos;
	o.UV = ip[i].UV;

	return o;
}
