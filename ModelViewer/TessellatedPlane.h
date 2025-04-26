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
	struct TransformCBuffer
	{
		DirectX::XMMATRIX Transform;
	};
	
	struct CameraCBuffer
	{
		DirectX::XMFLOAT3 CameraPos;
		float TessellationScale;
	};

	struct DomainCBuffer
	{
		DirectX::XMMATRIX ViewProj;
	};

	struct GeometryCBuffer
	{
		DirectX::XMFLOAT4 FrustumPlanes[6];
		DirectX::XMMATRIX FrustumCameraViewProj; // exposed seperately to view culling from a different camera
	};

	struct PlaneInfoCBuffer
	{
		float PlaneDimension;
		float HeightDisplacement;
		float Padding;
		BOOL bVisualiseChunks;
	};

public:
	TessellatedPlane() = delete;
	TessellatedPlane(UINT NumChunks, float ChunkSize, float TessellationScale, float HeightDisplacement);

	bool Init(const std::string& HeightMapFilepath);
	void Render();
	void Shutdown();

	virtual void RenderControls() override;

	void SetShouldRender(bool bNewShouldRender) { m_bShouldRender = bNewShouldRender; }
	bool ShouldRender() const { return m_bShouldRender; }

private:
	bool CreateShaders();
	bool CreateBuffers();

	void UpdateBuffers();

	void GenerateChunkTransforms(std::vector<DirectX::XMMATRIX>& ChunkTransforms);
	void PrepCullingBuffer(GeometryCBuffer& CullingBufferData, bool bNormalise = true);

private:
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_InputLayout;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> m_VertexShader;
	Microsoft::WRL::ComPtr<ID3D11HullShader> m_HullShader;
	Microsoft::WRL::ComPtr<ID3D11DomainShader> m_DomainShader;
	Microsoft::WRL::ComPtr<ID3D11GeometryShader> m_GeometryShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_PixelShader;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_IndexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_VertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_VertexCBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_HullCBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_DomainCBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_GeometryCBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_PlaneInfoCBuffer;

	ID3D11ShaderResourceView* m_HeightmapSRV;

	bool m_bShouldRender;
	bool m_bVisualiseChunks;
	float m_TessellationScale;
	float m_HeightDisplacement;
	UINT m_ChunkDimension;
	UINT m_NumChunks;

};

#endif
