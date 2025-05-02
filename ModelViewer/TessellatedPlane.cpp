#include "d3dcompiler.h"

#include "ImGui/imgui.h"

#include "TessellatedPlane.h"
#include "MyMacros.h"
#include "Graphics.h"
#include "Application.h"
#include "Shader.h"
#include "Camera.h"
#include "ResourceManager.h"
#include "Common.h"
#include "Landscape.h"

struct PlaneVertex
{
	DirectX::XMFLOAT3 Position;
};

const PlaneVertex ChunkVertices[] =
{
	{{-0.5f, 0.f, -0.5f}},
	{{-0.5f, 0.f,  0.5f}},
	{{ 0.5f, 0.f, -0.5f}},
	{{ 0.5f, 0.f,  0.5f}},
};

const UINT ChunkIndices[] =
{
	0, 1, 2, 3
};

TessellatedPlane::TessellatedPlane()
{
	SetName("Tessellated Plane");
	m_bShouldRender = true;
	m_HeightmapSRV = nullptr;
}

TessellatedPlane::~TessellatedPlane()
{
	Shutdown();
}

bool TessellatedPlane::Init(const std::string& HeightMapFilepath, float TessellationScale, Landscape* pLandscape)
{
	bool Result;
	FALSE_IF_FAILED(CreateShaders());
	FALSE_IF_FAILED(CreateBuffers());
	
	m_HeightmapSRV = ResourceManager::GetSingletonPtr()->LoadTexture(HeightMapFilepath);
	assert(m_HeightmapSRV);

	m_HeightMapFilepath = HeightMapFilepath;
	m_TessellationScale = TessellationScale;
	m_pLandscape = pLandscape;
	
	return true;
}

void TessellatedPlane::Render()
{
	Graphics::GetSingletonPtr()->EnableDepthWrite();
	Graphics::GetSingletonPtr()->DisableBlending();

	UpdateBuffers();

	Graphics* pGraphics = Graphics::GetSingletonPtr();
	ID3D11DeviceContext* DeviceContext = pGraphics->GetDeviceContext();
	UINT Strides[] = { sizeof(PlaneVertex), 0u };
	UINT Offsets[] = { 0u, 0u };
	ID3D11Buffer* Buffers[2] = { m_VertexBuffer.Get(), nullptr };

	DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);
	DeviceContext->IASetInputLayout(m_InputLayout.Get());
	DeviceContext->IASetVertexBuffers(0u, 2u, Buffers, Strides, Offsets);
	DeviceContext->IASetIndexBuffer(m_IndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0u);

	DeviceContext->VSSetShader(m_VertexShader, nullptr, 0u);
	DeviceContext->VSSetConstantBuffers(0u, 1u, m_pLandscape->m_ChunkTransformsCBuffer.GetAddressOf());
	DeviceContext->VSSetConstantBuffers(1u, 1u, m_pLandscape->m_LandscapeInfoCBuffer.GetAddressOf());
	DeviceContext->VSSetShaderResources(0u, 1u, &m_HeightmapSRV);
	DeviceContext->VSSetSamplers(0u, 1u, Graphics::GetSingletonPtr()->GetSamplerState().GetAddressOf());

	DeviceContext->HSSetShader(m_HullShader, nullptr, 0u);
	DeviceContext->HSSetConstantBuffers(0u, 1u, m_HullCBuffer.GetAddressOf());

	DeviceContext->DSSetShader(m_DomainShader, nullptr, 0u);
	DeviceContext->DSSetConstantBuffers(0u, 1u, m_pLandscape->m_CameraCBuffer.GetAddressOf());
	DeviceContext->DSSetConstantBuffers(1u, 1u, m_pLandscape->m_LandscapeInfoCBuffer.GetAddressOf());
	DeviceContext->DSSetShaderResources(0u, 1u, &m_HeightmapSRV);
	DeviceContext->DSSetSamplers(0u, 1u, Graphics::GetSingletonPtr()->GetSamplerState().GetAddressOf());

	DeviceContext->GSSetShader(m_GeometryShader, nullptr, 0u);
	DeviceContext->GSSetConstantBuffers(0u, 1u, m_pLandscape->m_CullingCBuffer.GetAddressOf());

	DeviceContext->PSSetShader(m_PixelShader, nullptr, 0u);
	DeviceContext->PSSetConstantBuffers(1u, 1u, m_pLandscape->m_LandscapeInfoCBuffer.GetAddressOf());
	DeviceContext->PSSetShaderResources(0u, 1u, &m_HeightmapSRV);
	DeviceContext->PSSetSamplers(0u, 1u, Graphics::GetSingletonPtr()->GetSamplerState().GetAddressOf());

	DeviceContext->Begin(pGraphics->GetPipelineStatsQuery().Get());
	DeviceContext->DrawIndexedInstanced(4u, m_pLandscape->m_NumChunks, 0u, 0u, 0u);
	DeviceContext->End(pGraphics->GetPipelineStatsQuery().Get());

	D3D11_QUERY_DATA_PIPELINE_STATISTICS Stats = {};
	while (DeviceContext->GetData(pGraphics->GetPipelineStatsQuery().Get(), &Stats, sizeof(Stats), 0) != S_OK)
	{
		// sleep or maybe do it on next frame
	}

	Application::GetSingletonPtr()->GetRenderStatsRef().TrianglesRendered.push_back(std::make_pair("Tessellated Plane", Stats.GSPrimitives));
	Application::GetSingletonPtr()->GetRenderStatsRef().InstancesRendered.push_back(std::make_pair("Tessellated Plane chunks", m_pLandscape->m_NumChunks)); // not yet doing frustum culling on chunks

	DeviceContext->HSSetShader(nullptr, nullptr, 0u);
	DeviceContext->DSSetShader(nullptr, nullptr, 0u);
	DeviceContext->GSSetShader(nullptr, nullptr, 0u);
}

void TessellatedPlane::Shutdown()
{
	m_InputLayout.Reset();
	m_IndexBuffer.Reset();
	m_VertexBuffer.Reset();
	m_HullCBuffer.Reset();

	ResourceManager::GetSingletonPtr()->UnloadShader<ID3D11VertexShader>(m_vsFilename);
	ResourceManager::GetSingletonPtr()->UnloadShader<ID3D11HullShader>(m_hsFilename);
	ResourceManager::GetSingletonPtr()->UnloadShader<ID3D11DomainShader>(m_dsFilename);
	ResourceManager::GetSingletonPtr()->UnloadShader<ID3D11GeometryShader>(m_gsFilename);
	ResourceManager::GetSingletonPtr()->UnloadShader<ID3D11PixelShader>(m_psFilename);

	ResourceManager::GetSingletonPtr()->UnloadTexture(m_HeightMapFilepath);
	m_HeightmapSRV = nullptr;
}

void TessellatedPlane::RenderControls()
{
	ImGui::Text(GetName().c_str());

	ImGui::DragFloat("Tessellation Scale", &m_TessellationScale, 0.1f, 1.f, 256.f, "%.f", ImGuiSliderFlags_AlwaysClamp);

	ImGui::Checkbox("Should Render Plane?", &m_bShouldRender);
}

bool TessellatedPlane::CreateShaders()
{
	HRESULT hResult;
	Microsoft::WRL::ComPtr<ID3D10Blob> vsBuffer;

	m_vsFilename = "Shaders/TessellatedPlaneVS.hlsl";
	m_hsFilename = "Shaders/TessellatedPlaneHS.hlsl";
	m_dsFilename = "Shaders/TessellatedPlaneDS.hlsl";
	m_gsFilename = "Shaders/TessellatedPlaneGS.hlsl";
	m_psFilename = "Shaders/TessellatedPlanePS.hlsl";

	m_VertexShader = ResourceManager::GetSingletonPtr()->LoadShader<ID3D11VertexShader>(m_vsFilename, "main", vsBuffer);
	m_HullShader = ResourceManager::GetSingletonPtr()->LoadShader<ID3D11HullShader>(m_hsFilename);
	m_DomainShader = ResourceManager::GetSingletonPtr()->LoadShader<ID3D11DomainShader>(m_dsFilename);
	m_GeometryShader = ResourceManager::GetSingletonPtr()->LoadShader<ID3D11GeometryShader>(m_gsFilename);
	m_PixelShader = ResourceManager::GetSingletonPtr()->LoadShader<ID3D11PixelShader>(m_psFilename);
	
	D3D11_INPUT_ELEMENT_DESC LayoutDesc[1] = {};
	LayoutDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	LayoutDesc[0].SemanticName = "POSITION";
	LayoutDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	HFALSE_IF_FAILED(Graphics::GetSingletonPtr()->GetDevice()->CreateInputLayout(LayoutDesc, _countof(LayoutDesc), vsBuffer->GetBufferPointer(), vsBuffer->GetBufferSize(), &m_InputLayout));
	NAME_D3D_RESOURCE(m_InputLayout, "Tessellated plane input layout");

	return true;
}

bool TessellatedPlane::CreateBuffers()
{
	HRESULT hResult;

	D3D11_BUFFER_DESC Desc = {};
	Desc.Usage = D3D11_USAGE_IMMUTABLE;
	Desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	Desc.ByteWidth = sizeof(ChunkIndices);

	D3D11_SUBRESOURCE_DATA Data = {};
	Data.pSysMem = ChunkIndices;

	HFALSE_IF_FAILED(Graphics::GetSingletonPtr()->GetDevice()->CreateBuffer(&Desc, &Data, &m_IndexBuffer));
	NAME_D3D_RESOURCE(m_IndexBuffer, "Tessellated plane index buffer");

	Desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	Desc.ByteWidth = sizeof(ChunkVertices);

	Data.pSysMem = ChunkVertices;

	HFALSE_IF_FAILED(Graphics::GetSingletonPtr()->GetDevice()->CreateBuffer(&Desc, &Data, &m_VertexBuffer));
	NAME_D3D_RESOURCE(m_VertexBuffer, "Tessellated plane vertex buffer");

	Desc.Usage = D3D11_USAGE_DYNAMIC;
	Desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	Desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	Desc.ByteWidth = sizeof(HullCBuffer);

	HFALSE_IF_FAILED(Graphics::GetSingletonPtr()->GetDevice()->CreateBuffer(&Desc, nullptr, &m_HullCBuffer));
	NAME_D3D_RESOURCE(m_HullCBuffer, "Tessellated plane hull constant buffer");
	
	return true;
}

void TessellatedPlane::UpdateBuffers()
{	
	HRESULT hResult;
	D3D11_MAPPED_SUBRESOURCE MappedResource;
	HullCBuffer* HullCBufferPtr;
	ID3D11DeviceContext* DeviceContext = Graphics::GetSingletonPtr()->GetDeviceContext();
	
	DirectX::XMFLOAT3 CameraPos = Application::GetSingletonPtr()->GetMainCamera()->GetPosition();
	
	ASSERT_NOT_FAILED(DeviceContext->Map(m_HullCBuffer.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource));
	HullCBufferPtr = (HullCBuffer*)MappedResource.pData;
	HullCBufferPtr->CameraPos = CameraPos;
	HullCBufferPtr->TessellationScale = m_TessellationScale;
	DeviceContext->Unmap(m_HullCBuffer.Get(), 0u);
}
