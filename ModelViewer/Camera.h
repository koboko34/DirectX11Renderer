#pragma once

#ifndef CAMERA_H
#define CAMERA_H

#include <DirectXMath.h>

#include "GameObject.h"

class Camera : public GameObject
{
public:
	Camera() = delete;
	Camera(const DirectX::XMMATRIX& Proj);

	virtual void RenderControls() override;

	virtual void SetRotation(float x, float y, float z) override;
	void SetLookDir(float, float, float);

	void CalcViewMatrix();

	DirectX::XMFLOAT3 GetLookDir() const { return m_LookDir; }
	DirectX::XMFLOAT3 GetRotatedLookDir() const;
	DirectX::XMFLOAT3 GetRotatedLookRight() const;

	void GetViewMatrix(DirectX::XMMATRIX& ViewMatrix) { ViewMatrix = m_ViewMatrix; }
	DirectX::XMMATRIX GetViewMatrix() const { return m_ViewMatrix; }
	void GetProjMatrix(DirectX::XMMATRIX& ProjMatrix) { ProjMatrix = m_ProjMatrix; }
	DirectX::XMMATRIX GetProjMatrix() const { return m_ProjMatrix; }

	bool ShouldVisualiseFrustum() const { return m_bVisualiseFrustum; }

private:
	DirectX::XMFLOAT3 m_LookDir;
	DirectX::XMMATRIX m_ViewMatrix;
	DirectX::XMMATRIX m_ProjMatrix;

	bool m_bVisualiseFrustum;

};

#endif
