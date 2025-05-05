#include <cstdlib>
#include <cwchar>
#include <cstring>
#include <fstream>

#include "d3dcompiler.h"

#include "FrustumCuller.h"
#include "Application.h"
#include "Graphics.h"
#include "MyMacros.h"
#include "Common.h"
#include "ResourceManager.h"

FrustumCuller::~FrustumCuller()
{
	Shutdown();
}

bool FrustumCuller::Init()
{
	Microsoft::WRL::ComPtr<ID3D10Blob> csBuffer;
	m_csFilename = "Shaders/FrustumCullingCS.hlsl";

	m_CullingShader = ResourceManager::GetSingletonPtr()->LoadShader<ID3D11ComputeShader>(m_csFilename, "FrustumCull");
	m_InstanceCountClearShader = ResourceManager::GetSingletonPtr()->LoadShader<ID3D11ComputeShader>(m_csFilename, "ClearInstanceCount");
	m_InstanceCountTransferShader = ResourceManager::GetSingletonPtr()->LoadShader<ID3D11ComputeShader>(m_csFilename, "TransferInstanceCount");

	bool Result;
	FALSE_IF_FAILED(CreateBuffers());
	FALSE_IF_FAILED(CreateBufferViews());

	return true;
}

void FrustumCuller::Shutdown()
{
	ResourceManager::GetSingletonPtr()->UnloadShader<ID3D11ComputeShader>(m_csFilename, "FrustumCull");
	ResourceManager::GetSingletonPtr()->UnloadShader<ID3D11ComputeShader>(m_csFilename, "ClearInstanceCount");
	ResourceManager::GetSingletonPtr()->UnloadShader<ID3D11ComputeShader>(m_csFilename, "TransferInstanceCount");
}

UINT FrustumCuller::GetInstanceCount()
{
	HRESULT hResult;
	D3D11_MAPPED_SUBRESOURCE Data = {};
	ID3D11DeviceContext* DeviceContext = Graphics::GetSingletonPtr()->GetDeviceContext();

	DeviceContext->CopyResource(m_StagingBuffer.Get(), m_InstanceCountBuffer.Get());
	
	ASSERT_NOT_FAILED(DeviceContext->Map(m_StagingBuffer.Get(), 0u, D3D11_MAP_READ, 0u, &Data));
	UINT InstanceCount = *static_cast<UINT*>(Data.pData);
	DeviceContext->Unmap(m_StagingBuffer.Get(), 0u);

	return InstanceCount;
}

void FrustumCuller::DispatchShader(const std::vector<DirectX::XMMATRIX>& Transforms, const std::vector<DirectX::XMFLOAT4>& Corners, const DirectX::XMMATRIX& ViewProj,
	const DirectX::XMMATRIX& ScaleMatrix)
{
	ID3D11DeviceContext* DeviceContext = Graphics::GetSingletonPtr()->GetDeviceContext();
	const UINT InitialCount = 0u;

	DeviceContext->CSSetUnorderedAccessViews(0u, 1u, m_CulledTransformsUAV.GetAddressOf(), &InitialCount);
	DeviceContext->CSSetUnorderedAccessViews(1u, 1u, m_InstanceCountBufferUAV.GetAddressOf(), nullptr);
	DeviceContext->CSSetShaderResources(0u, 1u, m_TransformsSRV.GetAddressOf());
	DeviceContext->CSSetConstantBuffers(0u, 1u, m_CBuffer.GetAddressOf());

	ClearInstanceCount();

	DeviceContext->CSSetShader(m_CullingShader, nullptr, 0u);

	// As each thread group will have 32 threads (as defined in shader), calculate how many thread groups we need using integer division
	UINT ThreadGroupCount = (UINT(Transforms.size()) + 31) / 32;

	UpdateBuffers(Transforms, Corners, ViewProj, ScaleMatrix, ThreadGroupCount);
	DeviceContext->Dispatch(ThreadGroupCount, 1u, 1u);
	Application::GetSingletonPtr()->GetRenderStatsRef().ComputeDispatches++;

	ID3D11Buffer* NullBuffers[] = { nullptr };
	ID3D11ShaderResourceView* NullSRVs[] = { nullptr };
	ID3D11UnorderedAccessView* NullUAVs[] = { nullptr, nullptr };
	DeviceContext->CSSetConstantBuffers(0u, 1u, NullBuffers);
	DeviceContext->CSSetShaderResources(0u, 1u, NullSRVs);
	DeviceContext->CSSetUnorderedAccessViews(0u, 2u, NullUAVs, nullptr);
	DeviceContext->CSSetShader(nullptr, nullptr, 0u);
}

void FrustumCuller::ClearInstanceCount()
{
	Graphics::GetSingletonPtr()->GetDeviceContext()->CSSetShader(m_InstanceCountClearShader, nullptr, 0u);
	Graphics::GetSingletonPtr()->GetDeviceContext()->Dispatch(1u, 1u, 1u);
	Application::GetSingletonPtr()->GetRenderStatsRef().ComputeDispatches++;
}

void FrustumCuller::SendInstanceCount(Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> ArgsBufferUAV)
{
	ID3D11DeviceContext* DeviceContext = Graphics::GetSingletonPtr()->GetDeviceContext();
	
	DeviceContext->CSSetShader(m_InstanceCountTransferShader, nullptr, 0u);
	DeviceContext->CSSetUnorderedAccessViews(1u, 1u, m_InstanceCountBufferUAV.GetAddressOf(), nullptr);
	DeviceContext->CSSetUnorderedAccessViews(2u, 1u, ArgsBufferUAV.GetAddressOf(), nullptr);

	DeviceContext->Dispatch(1u, 1u, 1u);
	Application::GetSingletonPtr()->GetRenderStatsRef().ComputeDispatches++;

	ID3D11UnorderedAccessView* NullUAVs[] = { nullptr, nullptr };
	DeviceContext->CSSetShader(nullptr, nullptr, 0u);
	DeviceContext->CSSetUnorderedAccessViews(1u, 2u, NullUAVs, nullptr);
}

bool FrustumCuller::CreateBuffers()
{
	HRESULT hResult;
	D3D11_BUFFER_DESC Desc = {};
	ID3D11Device* Device = Graphics::GetSingletonPtr()->GetDevice();

	Desc.Usage = D3D11_USAGE_DYNAMIC;
	Desc.ByteWidth = (UINT)(sizeof(DirectX::XMMATRIX) * MAX_INSTANCE_COUNT);
	Desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	Desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	Desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	Desc.StructureByteStride = sizeof(DirectX::XMMATRIX);

	HFALSE_IF_FAILED(Device->CreateBuffer(&Desc, nullptr, &m_TransformsBuffer));
	NAME_D3D_RESOURCE(m_TransformsBuffer, "Frustum culler transforms buffer");

	Desc.CPUAccessFlags = 0;
	Desc.Usage = D3D11_USAGE_DEFAULT;
	Desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;

	HFALSE_IF_FAILED(Device->CreateBuffer(&Desc, nullptr, &m_CulledTransformsBuffer));
	NAME_D3D_RESOURCE(m_CulledTransformsBuffer, "Frustum culler culled transforms buffer");

	Desc = {};
	Desc.Usage = D3D11_USAGE_DYNAMIC;
	Desc.ByteWidth = sizeof(CBufferData);
	Desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	Desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	HFALSE_IF_FAILED(Device->CreateBuffer(&Desc, nullptr, &m_CBuffer));
	NAME_D3D_RESOURCE(m_CBuffer, "Frustum culler constant buffer");

	Desc = {};
	Desc.Usage = D3D11_USAGE_STAGING;
	Desc.ByteWidth = sizeof(UINT);
	Desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

	HFALSE_IF_FAILED(Device->CreateBuffer(&Desc, nullptr, &m_StagingBuffer));
	NAME_D3D_RESOURCE(m_StagingBuffer, "Frustum culler staging buffer");

	D3D11_BUFFER_DESC InstanceBufferDesc = {};
	InstanceBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	InstanceBufferDesc.ByteWidth = sizeof(UINT);
	InstanceBufferDesc.StructureByteStride = sizeof(UINT);
	InstanceBufferDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	InstanceBufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;

	HFALSE_IF_FAILED(Device->CreateBuffer(&InstanceBufferDesc, nullptr, &m_InstanceCountBuffer));
	NAME_D3D_RESOURCE(m_InstanceCountBuffer, "Frustum culler instance count buffer");

	return true;
}

bool FrustumCuller::CreateBufferViews()
{
	HRESULT hResult;
	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	ID3D11Device* Device = Graphics::GetSingletonPtr()->GetDevice();

	uavDesc.Format = DXGI_FORMAT_UNKNOWN;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	uavDesc.Buffer.NumElements = (UINT)MAX_INSTANCE_COUNT;
	uavDesc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_APPEND;

	HFALSE_IF_FAILED(Device->CreateUnorderedAccessView(m_CulledTransformsBuffer.Get(), &uavDesc, &m_CulledTransformsUAV));
	NAME_D3D_RESOURCE(m_CulledTransformsUAV, "Frustum culler culled transforms buffer UAV");

	D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
	SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	SRVDesc.Format = DXGI_FORMAT_UNKNOWN;
	SRVDesc.Buffer.NumElements = (UINT)MAX_INSTANCE_COUNT;

	HFALSE_IF_FAILED(Device->CreateShaderResourceView(m_TransformsBuffer.Get(), &SRVDesc, &m_TransformsSRV));
	NAME_D3D_RESOURCE(m_TransformsSRV, "Frustum culler transforms buffer SRV");

	HFALSE_IF_FAILED(Device->CreateShaderResourceView(m_CulledTransformsBuffer.Get(), &SRVDesc, &m_CulledTransformsSRV));
	NAME_D3D_RESOURCE(m_CulledTransformsSRV, "Frustum culler culled transforms buffer SRV");

	uavDesc = {};
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	uavDesc.Format = DXGI_FORMAT_UNKNOWN;
	uavDesc.Buffer.NumElements = 1;

	HFALSE_IF_FAILED(Device->CreateUnorderedAccessView(m_InstanceCountBuffer.Get(), &uavDesc, &m_InstanceCountBufferUAV));
	NAME_D3D_RESOURCE(m_InstanceCountBufferUAV, "Frustum culler instance count buffer UAV");

	return true;
}

void FrustumCuller::UpdateBuffers(const std::vector<DirectX::XMMATRIX>& Transforms, const std::vector<DirectX::XMFLOAT4>& Corners, const DirectX::XMMATRIX& ViewProj,
	const DirectX::XMMATRIX& ScaleMatrix, UINT ThreadGroupCount)
{
	assert(Transforms.size() <= MAX_INSTANCE_COUNT);

	HRESULT hResult;
	CBufferData* CBufferDataPtr;
	ID3D11DeviceContext* DeviceContext = Graphics::GetSingletonPtr()->GetDeviceContext();
	D3D11_MAPPED_SUBRESOURCE MappedResource = {};

	ASSERT_NOT_FAILED(DeviceContext->Map(m_TransformsBuffer.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0u, &MappedResource));
	memcpy(MappedResource.pData, Transforms.data(), sizeof(DirectX::XMMATRIX) * Transforms.size());
	DeviceContext->Unmap(m_TransformsBuffer.Get(), 0u);

	ASSERT_NOT_FAILED(DeviceContext->Map(m_CBuffer.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0u, &MappedResource));
	CBufferDataPtr = (CBufferData*)MappedResource.pData;
	memcpy(CBufferDataPtr->Corners, Corners.data(), sizeof(DirectX::XMFLOAT4) * 8);
	CBufferDataPtr->ViewProj = DirectX::XMMatrixTranspose(ViewProj);
	CBufferDataPtr->ScaleMatrix = DirectX::XMMatrixTranspose(ScaleMatrix);
	CBufferDataPtr->SentInstanceCount = (UINT)Transforms.size();
	CBufferDataPtr->ThreadGroupCount[0] = ThreadGroupCount;
	CBufferDataPtr->ThreadGroupCount[1] = 1u;
	CBufferDataPtr->ThreadGroupCount[2] = 1u;
	DeviceContext->Unmap(m_CBuffer.Get(), 0u);
}