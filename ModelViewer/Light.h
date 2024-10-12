#pragma once

#ifndef LIGHT_H
#define LIGHT_H

#include <DirectXMath.h>

class Light
{
public:
	Light();
	Light(const Light&);
	~Light();

	void SetPosition(float x, float y, float z);
	void SetSpecularPower(float Power);
	void SetRadius(float Radius);

	DirectX::XMFLOAT3 GetPosition() const { return m_Position; }
	DirectX::XMFLOAT3* GetPositionPtr() { return &m_Position; }
	float GetSpecularPower() const { return m_SpecularPower; }
	float GetRadius() const { return m_Radius; }

private:
	DirectX::XMFLOAT3 m_Position;
	float m_SpecularPower;
	float m_Radius;
};

#endif
