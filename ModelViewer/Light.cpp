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

//////////////////////////////////////////////////////////////

DirectionalLight::DirectionalLight()
{
	SetDirection(1.f, -1.f, 1.f);
}

void DirectionalLight::SetDirection(float x, float y, float z)
{
	DirectX::XMFLOAT3 Dir = { x, y, z };
	DirectX::XMVECTOR v = DirectX::XMLoadFloat3(&Dir);
	v = DirectX::XMVector3Normalize(v);
	DirectX::XMStoreFloat3(&m_Dir, v);
}
