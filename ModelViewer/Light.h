#pragma once

#ifndef LIGHT_H
#define LIGHT_H

#include <DirectXMath.h>

#include "Component.h"

#include "GameObject.h"

class Light : public Component
{
public:
	Light();
	
	void SetDiffuseColor(float r, float g, float b);
	void SetSpecularPower(float Power);

	DirectX::XMFLOAT3 GetDiffuseColor() const { return m_DiffuseColor; }
	float GetSpecularPower() const { return m_SpecularPower; }

private:
	DirectX::XMFLOAT3 m_DiffuseColor;
	float m_SpecularPower;
	
};

class PointLight : public Light
{
public:
	PointLight();

	void SetRadius(float Radius);

	DirectX::XMFLOAT3 GetPosition() const { return GetOwner()->GetPosition(); }
	float GetRadius() const { return m_Radius; }

private:
	// DirectX::XMFLOAT3 m_Position; // can add this back when adding accumulated transform
	float m_Radius;

};

#endif
