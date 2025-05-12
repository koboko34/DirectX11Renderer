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
	DirectX::XMFLOAT2 Position;
};

const GrassVertex GrassVertices[] =
{
	{{-0.028f, 0.0f}},
	{{ 0.028f, 0.0f}},
	{{-0.026f, 0.2f}},
	{{ 0.026f, 0.2f}},
	{{-0.024f, 0.4f}},
	{{ 0.024f, 0.4f}},
	{{-0.022f, 0.6f}},
	{{ 0.022f, 0.6f}},
	{{-0.020f, 0.8f}},
	{{ 0.020f, 0.8f}},
	{{ 0.000f, 1.0f}},
};

const UINT GrassIndices[] =
{
	0, 1, 2,
	3, 4, 5,
	6, 7, 8,
	9, 10
};

Grass::Grass()
{
	SetName("Grass");
	SetWindDirection({ 1.f, 1.f });
	m_bShouldRender = true;
	m_Freq = 2.f;
	m_Amp = 1.5f;
	m_TimeScale = 6.f;
	m_FreqMultiplier = 1.4f;
	m_AmpMultiplier = 0.5f;
	m_WaveCount = 32u;
	m_WindStrength = 0.7f;
	m_SwayExponent = 1.5f;
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
	assert(m_GrassPerChunk <= MAX_GRASS_PER_CHUNK);

	FALSE_IF_FAILED(CreateBuffers());

	m_vsFilepath = "Shaders/GrassVS.hlsl";
	m_psFilepath = "Shaders/GrassPS.hlsl";
	m_VertexShader = ResourceManager::GetSingletonPtr()->LoadShader<ID3D11VertexShader>(m_vsFilepath, "main", vsBuffer);
	m_PixelShader = ResourceManager::GetSingletonPtr()->LoadShader<ID3D11PixelShader>(m_psFilepath);

	D3D11_INPUT_ELEMENT_DESC LayoutDesc[1] = {};
	LayoutDesc[0].Format = DXGI_FORMAT_R32G32_FLOAT;
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
	pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	pContext->IASetVertexBuffers(0u, 1u, m_VertexBuffer.GetAddressOf(), Strides, Offsets);

	ID3D11ShaderResourceView* vsSRVs[] = { m_pLandscape->m_HeightmapSRV, pApp->GetFrustumCuller()->GetCulledOffsetsSRV().Get(), m_GrassOffsetsBufferSRV.Get()};
	ID3D11Buffer* CBuffers[] = { m_pLandscape->m_LandscapeInfoCBuffer.Get(), m_pLandscape->m_CameraCBuffer.Get(), m_GrassCBuffer.Get() };
	pContext->VSSetShader(m_VertexShader, nullptr, 0u);
	pContext->VSSetShaderResources(0u, 3u, vsSRVs);
	pContext->VSSetConstantBuffers(0u, 3u, CBuffers);
	pContext->VSSetSamplers(0u, 1u, pGraphics->GetSamplerState().GetAddressOf());

	pContext->PSSetShader(m_PixelShader, nullptr, 0u);
	pContext->PSSetConstantBuffers(0u, 1u, m_pLandscape->m_LandscapeInfoCBuffer.GetAddressOf());

	pContext->DrawIndexedInstancedIndirect(m_ArgsBuffer.Get(), 0u);
	pApp->GetRenderStatsRef().DrawCalls++;

	UINT InstanceCount = m_pLandscape->m_ChunkInstanceCount * m_GrassPerChunk;
	pApp->GetRenderStatsRef().TrianglesRendered.push_back(std::make_pair("Grass", InstanceCount * (_countof(GrassVertices) - 2)));
	pApp->GetRenderStatsRef().InstancesRendered.push_back(std::make_pair("Grass", InstanceCount));

	ID3D11ShaderResourceView* NullSRVs[] = { nullptr, nullptr };
	pContext->VSSetShaderResources(0u, 2u, NullSRVs);
}

void Grass::RenderControls()
{
	ImGui::Text(GetName().c_str());

	ImGui::Checkbox("Should Render Grass?", &m_bShouldRender);

	ImGui::Dummy(ImVec2(0.f, 10.f));

	ImGui::Text("Wind");
	
	bool bDirty = false;
	if (ImGui::SliderFloat("Frequency", &m_Freq, 0.f, 100.f))
	{
		bDirty = true;
	}
	if (ImGui::SliderFloat("Amplitude", &m_Amp, 0.f, 5.f))
	{
		bDirty = true;
	}

	DirectX::XMFLOAT2 WindDir = m_WindDir;
	if (ImGui::SliderFloat2("Wind Direction", reinterpret_cast<float*>(&WindDir), -1.f, 1.f))
	{
		SetWindDirection(WindDir);
		bDirty = true;
	}
	if (ImGui::SliderFloat("Time Scale", &m_TimeScale, 0.f, 10.f))
	{
		bDirty = true;
	}
	if (ImGui::SliderFloat("Frequency Multiplier", &m_FreqMultiplier, 1.f, 5.f))
	{
		bDirty = true;
	}
	if (ImGui::SliderFloat("Amplitude Multiplier", &m_AmpMultiplier, 0.f, 1.f))
	{
		bDirty = true;
	}

	UINT WaveCountMin = 0u;
	UINT WaveCountMax = 64u;
	if (ImGui::SliderScalar("Wave Count", ImGuiDataType_U32, &m_WaveCount, &WaveCountMin, &WaveCountMax))
	{
		bDirty = true;
	}
	if (ImGui::SliderFloat("Sway Height Exponent", &m_SwayExponent, 1.f, 10.f))
	{
		bDirty = true;
	}
	if (ImGui::SliderFloat("Wind Strength", &m_WindStrength, 0.f, 3.f))
	{
		bDirty = true;
	}

	if (bDirty)
	{
		UpdateBuffers();
	}
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

	Desc.Usage = D3D11_USAGE_DYNAMIC;
	Desc.ByteWidth = sizeof(WindCBuffer);
	Desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	Desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	WindCBuffer WindData = {};
	WindData.Freq = m_Freq;
	WindData.Amp = m_Amp;
	WindData.Direction = m_WindDir;
	WindData.TimeScale = m_TimeScale;
	WindData.FreqMultiplier = m_FreqMultiplier;
	WindData.AmpMultiplier = m_AmpMultiplier;
	WindData.WaveCount = m_WaveCount;
	WindData.Strength = m_WindStrength;
	WindData.SwayExponent = m_SwayExponent;

	Data.pSysMem = &WindData;

	HFALSE_IF_FAILED(Graphics::GetSingletonPtr()->GetDevice()->CreateBuffer(&Desc, &Data, &m_GrassCBuffer));
	NAME_D3D_RESOURCE(m_GrassCBuffer, "Grass constant buffer");

	Desc.CPUAccessFlags = 0u;
	Desc.Usage = D3D11_USAGE_IMMUTABLE;
	Desc.ByteWidth = sizeof(DirectX::XMFLOAT2) * MAX_GRASS_PER_CHUNK; // TODO: can this just be m_GrassPerChunk?
	Desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	Desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	Desc.StructureByteStride = sizeof(DirectX::XMFLOAT2);

	Data.pSysMem = m_pLandscape->GetGrassPositions().data();

	HFALSE_IF_FAILED(Graphics::GetSingletonPtr()->GetDevice()->CreateBuffer(&Desc, &Data, &m_GrassOffsetsBuffer));
	NAME_D3D_RESOURCE(m_GrassOffsetsBuffer, "Grass offsets buffer");

	D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
	SRVDesc.Format = DXGI_FORMAT_UNKNOWN;
	SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	SRVDesc.Buffer.NumElements = (UINT)MAX_GRASS_PER_CHUNK; // same here?

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

void Grass::UpdateBuffers()
{
	HRESULT hResult;
	D3D11_MAPPED_SUBRESOURCE MappedResource;
	WindCBuffer* CBufferPtr;
	ID3D11DeviceContext* DeviceContext = Graphics::GetSingletonPtr()->GetDeviceContext();

	ASSERT_NOT_FAILED(DeviceContext->Map(m_GrassCBuffer.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource));
	CBufferPtr = (WindCBuffer*)MappedResource.pData;
	CBufferPtr->Freq = m_Freq;
	CBufferPtr->Amp = m_Amp;
	CBufferPtr->Direction = m_WindDir;
	CBufferPtr->TimeScale = m_TimeScale;
	CBufferPtr->FreqMultiplier = m_FreqMultiplier;
	CBufferPtr->AmpMultiplier = m_AmpMultiplier;
	CBufferPtr->WaveCount = m_WaveCount;
	CBufferPtr->Strength = m_WindStrength;
	CBufferPtr->SwayExponent = m_SwayExponent;
	DeviceContext->Unmap(m_GrassCBuffer.Get(), 0u);
}

void Grass::SetWindDirection(DirectX::XMFLOAT2 WindDir)
{
	DirectX::XMVECTOR v = DirectX::XMLoadFloat2(&WindDir);
	v = DirectX::XMVector2Normalize(v);
	DirectX::XMStoreFloat2(&m_WindDir, v);
}
