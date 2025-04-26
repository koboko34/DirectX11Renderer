cbuffer CullingBuffer
{
	float4 FrustumPlanes[6];  // left, right, top, bot, near, far
	float4x4 CullCameraViewProj;
};

struct GS_In
{
	float4 Pos : SV_POSITION;
	float3 WorldPos : WORLDPOS;
	float2 UV : TEXCOORD0;
	uint ChunkID : TEXCOORD1;
};

[maxvertexcount(3)]
void main(
	triangle GS_In input[3], 
	inout TriangleStream<GS_In> output
)
{
	bool bInsideFrustum = false;
	const float Bias = 0.5f;

	for (int i = 0; i < 3; ++i)
	{
		float4 Pos = mul(float4(input[i].WorldPos, 1.f), CullCameraViewProj);
		
		// check if at least one vertex is inside the clip space
		if (abs(Pos.x) <= Pos.w + Bias &&
            abs(Pos.y) <= Pos.w + Bias &&
            Pos.z >= -Bias && Pos.z <= Pos.w + Bias)
		{
			bInsideFrustum = true;
			break;
		}
	}

	if (bInsideFrustum)
	{
		for (int i = 0; i < 3; ++i)
		{
			output.Append(input[i]);
		}
	}
	
	/*bool bFullyOutside = false;
	
	for (int p = 0; p < 6; ++p)
	{
		int outsideCount = 0;

		for (int v = 0; v < 3; ++v)
		{
			float d = dot(FrustumPlanes[p], input[v].Pos);
			if (d < 0)
				outsideCount++;
		}

		if (outsideCount == 3)
		{
			bFullyOutside = true;
			break;
		}
	}
		
	if (!bFullyOutside)
	{
		output.Append(input[0]);
		output.Append(input[1]);
		output.Append(input[2]);
	}*/
}