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
