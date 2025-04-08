#include "Light.h"

Light::Light()
{
	m_DiffuseColor = { 1.f, 1.f, 1.f };
	m_SpecularPower = 256.f;
}

void Light::SetDiffuseColor(float r, float g, float b)
{
	m_DiffuseColor = DirectX::XMFLOAT3(r, g, b);
}

void Light::SetSpecularPower(float Power)
{
	m_SpecularPower = Power;
}

//////////////////////////////////////////////////////////////

PointLight::PointLight()
{
	m_Radius = 10.f;
}

void PointLight::SetRadius(float Radius)
{
	m_Radius = Radius;
}
