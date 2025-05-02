#pragma once

#ifndef TESSELLATED_PLANE_H
#define TESSELLATED_PLANE_H

#include "d3d11.h"
#include "DirectXMath.h"

#include "wrl.h"

#include "GameObject.h"

class TessellatedPlane : public GameObject
{
	friend class Landscape;

private:
	struct HullCBuffer
	{
		DirectX::XMFLOAT3 CameraPos;
		float TessellationScale;
	};

public:
	TessellatedPlane();
	~TessellatedPlane();

	bool Init(const std::string& HeightMapFilepath, float TessellationScale, Landscape* pLandscape);
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
	ID3D11VertexShader* m_VertexShader;
	ID3D11HullShader* m_HullShader;
	ID3D11DomainShader* m_DomainShader;
	ID3D11GeometryShader* m_GeometryShader;
	ID3D11PixelShader* m_PixelShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_InputLayout;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_IndexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_VertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_HullCBuffer;

	ID3D11ShaderResourceView* m_HeightmapSRV;
	std::string m_HeightMapFilepath;

	Landscape* m_pLandscape;
	float m_TessellationScale;
	bool m_bShouldRender;

	const char* m_vsFilename;
	const char* m_hsFilename;
	const char* m_dsFilename;
	const char* m_gsFilename;
	const char* m_psFilename;

};

#endif
