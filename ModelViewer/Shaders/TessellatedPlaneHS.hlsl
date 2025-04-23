struct HS_In
{
	float3 vPosition : POSITION;
};

struct HS_Out
{
	float3 vPosition : POSITION;
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
	TessFactor = lerp(1.f, 64.f, TessFactor);
	return Quantize(TessFactor);
}

HS_CONSTANT_DATA_OUTPUT CalcHSPatchConstants(
	InputPatch<HS_In, NUM_CONTROL_POINTS> ip,
	uint PatchID : SV_PrimitiveID)
{
	HS_CONSTANT_DATA_OUTPUT Output;
	
	float Tess0 = GetEdgeTessFactor(ip[0].vPosition, ip[1].vPosition);
	float Tess1 = GetEdgeTessFactor(ip[1].vPosition, ip[2].vPosition);
	float Tess2 = GetEdgeTessFactor(ip[2].vPosition, ip[3].vPosition);
	float Tess3 = GetEdgeTessFactor(ip[3].vPosition, ip[0].vPosition);
	
	float InnerTess = max(max(Tess0, Tess1), max(Tess2, Tess3));

	Output.EdgeTessFactor[0] = Tess0;
	Output.EdgeTessFactor[1] = Tess1;
	Output.EdgeTessFactor[2] = Tess2;
	Output.EdgeTessFactor[3] = Tess3;
	
	Output.InsideTessFactor[0] = InnerTess;
	Output.InsideTessFactor[1] = InnerTess;

	return Output;
}

[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(NUM_CONTROL_POINTS)]
[patchconstantfunc("CalcHSPatchConstants")]
HS_Out main( 
	InputPatch<HS_In, NUM_CONTROL_POINTS> ip, 
	uint i : SV_OutputControlPointID,
	uint PatchID : SV_PrimitiveID )
{
	HS_Out Output;

	Output.vPosition = ip[i].vPosition;

	return Output;
}
