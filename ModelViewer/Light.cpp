#include "Light.h"

Light::Light()
{
}

Light::Light(const Light&)
{
}

Light::~Light()
{
}

void Light::SetPosition(float x, float y, float z)
{
	m_Position = DirectX::XMFLOAT3(x, y, z);
}

void Light::SetSpecularPower(float Power)
{
	m_SpecularPower = Power;
}

void Light::SetRadius(float Radius)
{
	m_Radius = Radius;
}
