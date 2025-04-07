#pragma once

#ifndef CAMERA_H
#define CAMERA_H

#include <DirectXMath.h>

class Camera
{
public:
	Camera();
	Camera(const Camera&);
	~Camera();

	void SetPosition(float, float, float);
	void SetRotation(float Pitch, float Yaw);
	void SetLookDir(float, float, float);

	void CalcViewMatrix();

private:
	float m_PositionX, m_PositionY, m_PositionZ;
	float m_RotationX, m_RotationY, m_RotationZ;
	DirectX::XMFLOAT3 m_LookDir;

	DirectX::XMMATRIX m_ViewMatrix;

public:
	DirectX::XMFLOAT3 GetPosition() const { return DirectX::XMFLOAT3(m_PositionX, m_PositionY, m_PositionZ); }
	DirectX::XMFLOAT3 GetRotation() const { return DirectX::XMFLOAT3(m_RotationX, m_RotationY, m_RotationZ); }
	DirectX::XMFLOAT3 GetLookDir() const { return m_LookDir; }
	DirectX::XMFLOAT3 GetRotatedLookDir() const;
	DirectX::XMFLOAT3 GetRotatedLookRight() const;
	void GetViewMatrix(DirectX::XMMATRIX& ViewMatrix) { ViewMatrix = m_ViewMatrix; }
	DirectX::XMMATRIX GetViewMatrix() const { return m_ViewMatrix; }
};

#endif
