#include "Light.h"

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
