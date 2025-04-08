#include "Light.h"

Light::Light()
{
	m_Position = { 0.f, 0.f, 0.f };
	m_DiffuseColor = { 1.f, 1.f, 1.f };
	m_SpecularPower = 256.f;
	m_Radius = 10.f;
}

void Light::SetPosition(float x, float y, float z)
{
	m_Position = DirectX::XMFLOAT3(x, y, z);
}

void Light::SetDiffuseColor(float r, float g, float b)
{
	m_DiffuseColor = DirectX::XMFLOAT3(r, g, b);
}

void Light::SetSpecularPower(float Power)
{
	m_SpecularPower = Power;
}

void Light::SetRadius(float Radius)
{
	m_Radius = Radius;
}
