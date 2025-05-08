#include "ImGui/imgui.h"

#include "Grass.h"
#include "ResourceManager.h"
#include "MyMacros.h"
#include "Graphics.h"
#include "Application.h"
#include "Landscape.h"
#include "FrustumCuller.h"

typedef unsigned int UINT;

struct GrassVertex
{
	DirectX::XMFLOAT3 Position;
};

const GrassVertex GrassVertices[] =
{
	{{-0.03f, 0.f,  0.f}},
	{{ 0.0f,  1.f,  0.f}},
	{{ 0.03f, 0.f,  0.f}},
};

const UINT GrassIndices[] =
{
	0, 1, 2
};

Grass::Grass()
{
	SetName("Grass");
	m_bShouldRender = true;

	m_pLandscape = nullptr;
}

Grass::~Grass()
{
	Shutdown();
}

bool Grass::Init(Landscape* pLandscape, UINT GrassDimensionPerChunk)
{
	HRESULT hResult;
	bool Result;
	Microsoft::WRL::ComPtr<ID3D10Blob> vsBuffer;
	m_pLandscape = pLandscape;
	m_GrassPerChunk = GrassDimensionPerChunk * GrassDimensionPerChunk;

	FALSE_IF_FAILED(CreateBuffers());

	m_vsFilepath = "Shaders/GrassVS.hlsl";
	m_psFilepath = "Shaders/GrassPS.hlsl";
	m_VertexShader = ResourceManager::GetSingletonPtr()->LoadShader<ID3D11VertexShader>(m_vsFilepath, "main", vsBuffer);
	m_PixelShader = ResourceManager::GetSingletonPtr()->LoadShader<ID3D11PixelShader>(m_psFilepath);

	D3D11_INPUT_ELEMENT_DESC LayoutDesc[1] = {};
	LayoutDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	LayoutDesc[0].SemanticName = "POSITION";
	LayoutDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	HFALSE_IF_FAILED(Graphics::GetSingletonPtr()->GetDevice()->CreateInputLayout(LayoutDesc, _countof(LayoutDesc), vsBuffer->GetBufferPointer(), vsBuffer->GetBufferSize(), &m_InputLayout));
	NAME_D3D_RESOURCE(m_InputLayout, "Grass input layout");

	return true;
}

void Grass::Shutdown()
{
	ResourceManager::GetSingletonPtr()->UnloadShader<ID3D11VertexShader>(m_vsFilepath);
	ResourceManager::GetSingletonPtr()->UnloadShader<ID3D11PixelShader>(m_psFilepath);
}

void Grass::Render()
{
	Application* pApp = Application::GetSingletonPtr();
	Graphics* pGraphics = Graphics::GetSingletonPtr();
	ID3D11DeviceContext* pContext = pGraphics->GetDeviceContext();
	pGraphics->SetRasterStateBackFaceCull(false);

	pApp->GetFrustumCuller()->SendInstanceCount(m_ArgsBufferUAV, m_GrassPerChunk);

	UINT Strides[] = { sizeof(GrassVertex) };
	UINT Offsets[] = { 0u };
	pContext->IASetInputLayout(m_InputLayout.Get());
	pContext->IASetIndexBuffer(m_IndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0u);
	pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pContext->IASetVertexBuffers(0u, 1u, m_VertexBuffer.GetAddressOf(), Strides, Offsets);

	ID3D11ShaderResourceView* vsSRVs[] = { m_pLandscape->m_HeightmapSRV, pApp->GetFrustumCuller()->GetCulledTransformsSRV().Get(), m_GrassOffsetsBufferSRV.Get()};
	ID3D11Buffer* CBuffers[] = { m_pLandscape->m_LandscapeInfoCBuffer.Get(), m_pLandscape->m_CameraCBuffer.Get() };
	pContext->VSSetShader(m_VertexShader, nullptr, 0u);
	pContext->VSSetShaderResources(0u, 3u, vsSRVs);
	pContext->VSSetConstantBuffers(0u, 2u, CBuffers);
	pContext->VSSetSamplers(0u, 1u, pGraphics->GetSamplerState().GetAddressOf());

	pContext->PSSetShader(m_PixelShader, nullptr, 0u);

	pContext->DrawIndexedInstancedIndirect(m_ArgsBuffer.Get(), 0u);
	pApp->GetRenderStatsRef().DrawCalls++;

	UINT InstanceCount = m_pLandscape->m_ChunkInstanceCount * (UINT)m_pLandscape->GetGrassPositions().size();
	pApp->GetRenderStatsRef().TrianglesRendered.push_back(std::make_pair("Grass", InstanceCount * (_countof(GrassIndices) / 3))); // currently only using 1 grass model which is 1 triangle each
	pApp->GetRenderStatsRef().InstancesRendered.push_back(std::make_pair("Grass", InstanceCount));

	ID3D11ShaderResourceView* NullSRVs[] = { nullptr, nullptr };
	pContext->VSSetShaderResources(0u, 2u, NullSRVs);
}

void Grass::RenderControls()
{
	ImGui::Text(GetName().c_str());

	ImGui::Checkbox("Should Render Grass?", &m_bShouldRender);
}

bool Grass::CreateBuffers()
{
	HRESULT hResult;
	D3D11_BUFFER_DESC Desc = {};
	Desc.Usage = D3D11_USAGE_IMMUTABLE;
	Desc.ByteWidth = sizeof(GrassVertices);
	Desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA Data = {};
	Data.pSysMem = GrassVertices;

	HFALSE_IF_FAILED(Graphics::GetSingletonPtr()->GetDevice()->CreateBuffer(&Desc, &Data, &m_VertexBuffer));
	NAME_D3D_RESOURCE(m_VertexBuffer, "Grass vertex buffer");

	Desc.ByteWidth = sizeof(GrassIndices);
	Desc.BindFlags = D3D11_BIND_INDEX_BUFFER;

	Data.pSysMem = GrassIndices;

	HFALSE_IF_FAILED(Graphics::GetSingletonPtr()->GetDevice()->CreateBuffer(&Desc, &Data, &m_IndexBuffer));
	NAME_D3D_RESOURCE(m_IndexBuffer, "Grass index buffer");

	Desc.ByteWidth = sizeof(DirectX::XMMATRIX) * MAX_GRASS_PER_CHUNK;
	Desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	Desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	Desc.StructureByteStride = sizeof(DirectX::XMMATRIX);

	Data.pSysMem = m_pLandscape->GetGrassPositions().data();

	HFALSE_IF_FAILED(Graphics::GetSingletonPtr()->GetDevice()->CreateBuffer(&Desc, &Data, &m_GrassOffsetsBuffer));
	NAME_D3D_RESOURCE(m_GrassOffsetsBuffer, "Grass offsets buffer");

	D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
	SRVDesc.Format = DXGI_FORMAT_UNKNOWN;
	SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	SRVDesc.Buffer.NumElements = (UINT)MAX_GRASS_PER_CHUNK;

	HFALSE_IF_FAILED(Graphics::GetSingletonPtr()->GetDevice()->CreateShaderResourceView(m_GrassOffsetsBuffer.Get(), &SRVDesc, &m_GrassOffsetsBufferSRV));
	NAME_D3D_RESOURCE(m_GrassOffsetsBufferSRV, "Grass offsets buffer SRV");

	Desc = {};
	Desc.ByteWidth = sizeof(D3D11_DRAW_INDEXED_INSTANCED_INDIRECT_ARGS);
	Desc.Usage = D3D11_USAGE_DEFAULT;
	Desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
	Desc.MiscFlags = D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS | D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;

	D3D11_DRAW_INDEXED_INSTANCED_INDIRECT_ARGS ArgsData;
	ArgsData.IndexCountPerInstance = sizeof(GrassIndices) / sizeof(UINT);
	ArgsData.InstanceCount = 0u;
	ArgsData.StartIndexLocation = 0u;
	ArgsData.BaseVertexLocation = 0;
	ArgsData.StartInstanceLocation = 0u;

	Data = {};
	Data.pSysMem = &ArgsData;

	HFALSE_IF_FAILED(Graphics::GetSingletonPtr()->GetDevice()->CreateBuffer(&Desc, &Data, &m_ArgsBuffer));
	NAME_D3D_RESOURCE(m_ArgsBuffer, "Grass args buffer");

	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	uavDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	uavDesc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_RAW;
	uavDesc.Buffer.NumElements = sizeof(D3D11_DRAW_INDEXED_INSTANCED_INDIRECT_ARGS) / 4;

	HFALSE_IF_FAILED(Graphics::GetSingletonPtr()->GetDevice()->CreateUnorderedAccessView(m_ArgsBuffer.Get(), &uavDesc, &m_ArgsBufferUAV));
	NAME_D3D_RESOURCE(m_ArgsBufferUAV, "Grass args buffer UAV");
	
	return true;
}
