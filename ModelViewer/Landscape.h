#pragma once

#ifndef LANDSCAPE_H
#define LANDSCAPE_H

#include "d3d11.h"
#include "DirectXMath.h"

#include "wrl.h"

#include "GameObject.h"

class Landscape : public GameObject
{
	friend class TessellatedPlane;

private:
	struct TransformCBuffer
	{
		DirectX::XMMATRIX Transform;
	};
	
	struct CameraCBuffer
	{
		DirectX::XMMATRIX ViewProj;
	};

	struct CullingCBuffer
	{
		DirectX::XMFLOAT4 FrustumPlanes[6];
		DirectX::XMMATRIX FrustumCameraViewProj; // exposed seperately to view culling from a different camera
	};

	struct LandscapeInfoCBuffer
	{
		float PlaneDimension;
		float HeightDisplacement;
		float Padding;
		BOOL bVisualiseChunks;
	};

public:
	Landscape() = delete;
	Landscape(UINT NumChunks, float ChunkSize, float HeightDisplacement);
	~Landscape();

	bool Init(const std::string& HeightMapFilepath, float TessellationScale);
	void Render();
	void Shutdown();

	virtual void RenderControls() override;

	void SetShouldRender(bool bNewShouldRender) { m_bShouldRender = bNewShouldRender; }
	bool ShouldRender() const { return m_bShouldRender; }

private:
	bool CreateBuffers();
	void UpdateBuffers();

	void GenerateChunkTransforms(std::vector<DirectX::XMMATRIX>& ChunkTransforms);
	void PrepCullingBuffer(CullingCBuffer& CullingBufferData, bool bNormalise = true);

private:
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_ChunkTransformsCBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_LandscapeInfoCBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_CullingCBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_CameraCBuffer;

	std::unique_ptr<TessellatedPlane> m_Plane;

	bool m_bShouldRender;
	bool m_bVisualiseChunks;
	float m_ChunkSize;
	float m_HeightDisplacement;
	UINT m_ChunkDimension;
	UINT m_NumChunks;

};

#endif
