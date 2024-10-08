cbuffer MatrixBuffer
{
	matrix WorldMatrix;
	matrix ViewMatrix;
	matrix ProjectionMatrix;
};

struct VS_In
{
	float4 Pos : POSITION;
};

struct VS_Out
{
	float4 Pos : SV_POSITION;
};

VS_Out main(VS_In v)
{
	VS_Out o;
	
	o.Pos = mul(v.Pos, WorldMatrix);
	
	return o;
}