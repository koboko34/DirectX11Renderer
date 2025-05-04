#pragma once

#ifndef BOX_RENDERER_H
#define BOX_RENDERER_H

#include <memory>
#include <vector>

#include "d3d11.h"
#include "DirectXMath.h"

#include "wrl.h"

class Camera;
struct AABB;

class BoxRenderer
{
public:
	BoxRenderer() {}
	~BoxRenderer();

	bool Init();
	void Shutdown();

	void RenderBox(const AABB& BBox, const DirectX::XMMATRIX& Transform);
	void RenderFrustum(const std::shared_ptr<Camera>& pCamera);

private:
	bool CreateShaders();
	bool CreateBuffers();

	void Render();

	void LoadBoxCorners(const AABB& BBox, const DirectX::XMMATRIX& Transform);
	void LoadFrustumCorners(const std::shared_ptr<Camera>& pCamera);
	void UpdateBuffers();

private:
	ID3D11VertexShader* m_VertexShader;
	ID3D11PixelShader* m_PixelShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_InputLayout;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_IndexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_VertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_VertexCBuffer;

	std::vector<DirectX::XMFLOAT3> m_BoxCorners;

	const char* m_vsFilename;
	const char* m_psFilename;

};

#endif
