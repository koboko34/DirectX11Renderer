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
	float Padding;
};

#define NUM_CONTROL_POINTS 4

HS_CONSTANT_DATA_OUTPUT CalcHSPatchConstants(
	InputPatch<HS_In, NUM_CONTROL_POINTS> ip,
	uint PatchID : SV_PrimitiveID)
{
	HS_CONSTANT_DATA_OUTPUT Output;

	Output.EdgeTessFactor[0] = 64.f;
	Output.EdgeTessFactor[1] = 64.f;
	Output.EdgeTessFactor[2] = 64.f;
	Output.EdgeTessFactor[3] = 64.f;
	
	Output.InsideTessFactor[0] = 64.f;
	Output.InsideTessFactor[1] = 64.f;

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
