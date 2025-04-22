#pragma once

#ifndef TESSELLATED_PLANE_H
#define TESSELLATED_PLANE_H

#include "d3d11.h"
#include "DirectXMath.h"

#include "wrl.h"

#include "GameObject.h"

class TessellatedPlane : public GameObject
{
private:
	struct TransformBuffer
	{
		DirectX::XMMATRIX Transform;
	};
	
	struct CameraBuffer
	{
		DirectX::XMFLOAT3 CameraPos;
		float Padding;
	};

	struct MatrixBuffer
	{
		DirectX::XMMATRIX ViewProj;
	};


public:
	TessellatedPlane();

	bool Init();
	void Render();
	void Shutdown();

	virtual void RenderControls() override;

	void SetShouldRender(bool bNewShouldRender) { m_bShouldRender = bNewShouldRender; }
	bool ShouldRender() const { return m_bShouldRender; }

private:
	bool CreateShaders();
	bool CreateBuffers();

	void UpdateBuffers();

private:
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_InputLayout;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> m_VertexShader;
	Microsoft::WRL::ComPtr<ID3D11HullShader> m_HullShader;
	Microsoft::WRL::ComPtr<ID3D11DomainShader> m_DomainShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_PixelShader;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_VertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_VertexConstantBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_HullConstantBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_DomainConstantBuffer;

	bool m_bShouldRender;

};

#endif
