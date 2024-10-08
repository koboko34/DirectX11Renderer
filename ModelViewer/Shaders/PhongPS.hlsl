struct PS_In
{
	float4 Pos : SV_POSITION;
};

float4 main(PS_In p) : SV_TARGET
{
	return float4(1.0f, 1.0f, 1.0f, 1.0f);
}