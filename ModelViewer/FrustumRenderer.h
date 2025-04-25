#pragma once

#ifndef FRUSTUM_RENDERER_H
#define FRUSTUM_RENDERER_H

#include <memory>
#include <vector>

#include "d3d11.h"
#include "DirectXMath.h"

#include "wrl.h"

class Camera;

class FrustumRenderer
{
public:
	FrustumRenderer() {}

	bool Init();

	void RenderFrustum(const std::shared_ptr<Camera>& pCamera);

private:
	bool CreateShaders();
	bool CreateBuffers();

	void LoadFrustumCorners(const std::shared_ptr<Camera>& pCamera);
	void UpdateBuffers(const std::shared_ptr<Camera>& pCamera);

private:
	Microsoft::WRL::ComPtr<ID3D11VertexShader> m_VertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_PixelShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_InputLayout;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_IndexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_VertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_VertexCBuffer;

	std::vector<DirectX::XMFLOAT3> m_FrustumCorners;

};

#endif
