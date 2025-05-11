// if changing these, also update in Common.h

#define MAX_POINT_LIGHTS 8
#define MAX_DIRECTIONAL_LIGHTS 1
#define MAX_PLANE_CHUNKS 1024
#define MAX_GRASS_PER_CHUNK 10000
#define MAX_INSTANCE_COUNT 1024

float Remap(float Value, float FromMin, float FromMax, float ToMin, float ToMax)
{
	return ToMin + (Value - FromMin) * (ToMax - ToMin) / (FromMax - FromMin);
}

float2 GetHeightmapUV(float3 Pos, float PlaneDimension)
{
	float HalfPlaneDimension = PlaneDimension / 2.f;
	float x = Remap(Pos.x, -HalfPlaneDimension, HalfPlaneDimension, 0.f, 1.f);
	float z = Remap(Pos.z, -HalfPlaneDimension, HalfPlaneDimension, 0.f, 1.f);
	z = 1.f - z;
	
	return float2(x, z);
}

uint GenerateChunkID(float2 v)
{
	int2 i = int2(v * 65536.0f);
	uint hash = uint(i.x) * 73856093u ^ uint(i.y) * 19349663u;
	hash ^= (hash >> 13);
	hash *= 0x85ebca6bu;
	hash ^= (hash >> 16);

	return hash;
}

float2 __fade(float2 t)
{
	return t * t * t * (t * (t * 6 - 15) + 10);
}

float2 __hash(float2 p)
{
    // Simple hash to generate pseudo-random gradients
	p = float2(dot(p, float2(127.1, 311.7)),
               dot(p, float2(269.5, 183.3)));
	return frac(sin(p) * 43758.5453);
}

float __grad(float2 hash, float2 pos)
{
    // Convert hash to gradient direction (-1 to 1)
	float2 grad = normalize(hash * 2.0 - 1.0);
	return dot(grad, pos);
}

float PerlinNoise(float2 p)
{
	float2 pi = floor(p);
	float2 pf = frac(p);

    // Get hash gradients for the 4 corners
	float2 h00 = __hash(pi + float2(0.0, 0.0));
	float2 h10 = __hash(pi + float2(1.0, 0.0));
	float2 h01 = __hash(pi + float2(0.0, 1.0));
	float2 h11 = __hash(pi + float2(1.0, 1.0));

    // Compute dot products
	float d00 = __grad(h00, pf - float2(0.0, 0.0));
	float d10 = __grad(h10, pf - float2(1.0, 0.0));
	float d01 = __grad(h01, pf - float2(0.0, 1.0));
	float d11 = __grad(h11, pf - float2(1.0, 1.0));

    // Interpolate
	float2 f = __fade(pf);
	float x1 = lerp(d00, d10, f.x);
	float x2 = lerp(d01, d11, f.x);
	return lerp(x1, x2, f.y);
}
