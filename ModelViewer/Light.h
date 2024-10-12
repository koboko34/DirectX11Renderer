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
	void SetDiffuseColor(float r, float g, float b);
	void SetSpecularPower(float Power);
	void SetRadius(float Radius);

	DirectX::XMFLOAT3 GetPosition() const { return m_Position; }
	DirectX::XMFLOAT3* GetPositionPtr() { return &m_Position; }
	DirectX::XMFLOAT3 GetDiffuseColor() const { return m_DiffuseColor; }
	float GetSpecularPower() const { return m_SpecularPower; }
	float GetRadius() const { return m_Radius; }

private:
	DirectX::XMFLOAT3 m_Position;
	DirectX::XMFLOAT3 m_DiffuseColor;
	float m_SpecularPower;
	float m_Radius;
	
};

#endif
