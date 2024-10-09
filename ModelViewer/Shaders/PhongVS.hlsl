cbuffer MatrixBuffer
{
	matrix WorldMatrix;
	matrix ViewMatrix;
	matrix ProjectionMatrix;
};

struct VS_In
{
	float4 Pos : POSITION;
	float2 TexCoord : TEXCOORD0;
	float4 Normal : NORMAL;
};

struct VS_Out
{
	float4 Pos : SV_POSITION;
	float2 TexCoord : TEXCOORD0;
	float4 Normal : NORMAL;
};

VS_Out main(VS_In v)
{
	VS_Out o;
	
	v.Pos.w = 1.f;	
	
	o.Pos = mul(v.Pos, WorldMatrix);
	o.Pos = mul(o.Pos, ViewMatrix);
	o.Pos = mul(o.Pos, ProjectionMatrix);
	
	o.TexCoord = v.TexCoord;
	o.Normal = v.Normal;
	
	return o;
}