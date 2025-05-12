#pragma once

#ifndef FRUSTUM_CULLER_H
#define FRUSTUM_CULLER_H

#include <vector>

#include "DirectXMath.h"
#include "d3d11.h"

#include "wrl.h"

class FrustumCuller
{
private:
	struct CBufferData
	{
		DirectX::XMFLOAT4 Corners[8];
		DirectX::XMMATRIX ScaleMatrix;
		DirectX::XMMATRIX ViewProj;
		UINT SentInstanceCount;
		UINT ThreadGroupCount[3];
	};

	struct InstanceCountMultiplierBufferData
	{
		UINT Multiplier;
		DirectX::XMFLOAT3 Padding;
	};

public:
	FrustumCuller() = default;
	~FrustumCuller();

	bool Init();
	void Shutdown();

	void DispatchShader(const std::vector<DirectX::XMMATRIX>& Transforms, const std::vector<DirectX::XMFLOAT4>& Corners, const DirectX::XMMATRIX& ViewProj,
		const DirectX::XMMATRIX& ScaleMatrix = DirectX::XMMatrixIdentity());
	void DispatchShader(const std::vector<DirectX::XMFLOAT2>& Offsets, const std::vector<DirectX::XMFLOAT4>& Corners, const DirectX::XMMATRIX& ViewProj,
		const DirectX::XMMATRIX& ScaleMatrix = DirectX::XMMatrixIdentity());
	void ClearInstanceCount();
	void SendInstanceCount(Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> ArgsBufferUAV, UINT InstanceCountMultiplier = 1u);
	UINT GetInstanceCount();

	Microsoft::WRL::ComPtr<ID3D11Buffer> GetCulledTransformsBuffer() const { return m_CulledTransformsBuffer; }
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> GetCulledTransformsSRV() const { return m_CulledTransformsSRV; }
	Microsoft::WRL::ComPtr<ID3D11Buffer> GetCulledOffsetsBuffer() const { return m_CulledOffsetsBuffer; }
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> GetCulledOffsetsSRV() const { return m_CulledOffsetsSRV; }

private:
	bool CreateBuffers();
	bool CreateBufferViews();

	void UpdateBuffers(const std::vector<DirectX::XMMATRIX>& Transforms, const std::vector<DirectX::XMFLOAT4>& Corners, const DirectX::XMMATRIX& ViewProj,
		const DirectX::XMMATRIX& ScaleMatrix, UINT ThreadGroupCount);
	void UpdateBuffers(const std::vector<DirectX::XMFLOAT2>& Offsets, const std::vector<DirectX::XMFLOAT4>& Corners, const DirectX::XMMATRIX& ViewProj,
		const DirectX::XMMATRIX& ScaleMatrix, UINT ThreadGroupCount);
	void UpdateCBuffer(const std::vector<DirectX::XMFLOAT4>& Corners, const DirectX::XMMATRIX& ViewProj,
		const DirectX::XMMATRIX& ScaleMatrix, UINT ThreadGroupCount, UINT SentInstanceCount);

	void DispatchShaderImpl(UINT ThreadGroupCount);

private:
	ID3D11ComputeShader* m_CullingShader;
	ID3D11ComputeShader* m_OffsetsCullingShader;
	ID3D11ComputeShader* m_InstanceCountClearShader;
	ID3D11ComputeShader* m_InstanceCountTransferShader;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_TransformsBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_OffsetsBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_CulledTransformsBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_CulledOffsetsBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_CBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_InstanceCountMultiplierCBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_StagingBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_InstanceCountBuffer;
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> m_InstanceCountBufferUAV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_TransformsSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_OffsetsSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_CulledTransformsSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_CulledOffsetsSRV;
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> m_CulledTransformsUAV;
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> m_CulledOffsetsUAV;

	const char* m_csFilename;
	bool m_bGotInstanceCount;
};

#endif
