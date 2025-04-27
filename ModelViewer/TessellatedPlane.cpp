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

	ID3D11DeviceContext* DeviceContext = Graphics::GetSingletonPtr()->GetDeviceContext();
	UINT Strides[] = { sizeof(PlaneVertex), 0u };
	UINT Offsets[] = { 0u, 0u };
	ID3D11Buffer* Buffers[2] = { m_VertexBuffer.Get(), nullptr };

	DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);
	DeviceContext->IASetInputLayout(m_InputLayout.Get());
	DeviceContext->IASetVertexBuffers(0u, 2u, Buffers, Strides, Offsets);
	DeviceContext->IASetIndexBuffer(m_IndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0u);

	DeviceContext->VSSetShader(m_VertexShader.Get(), nullptr, 0u);
	DeviceContext->VSSetConstantBuffers(0u, 1u, m_pLandscape->m_ChunkTransformsCBuffer.GetAddressOf());
	DeviceContext->VSSetConstantBuffers(1u, 1u, m_pLandscape->m_LandscapeInfoCBuffer.GetAddressOf());
	DeviceContext->VSSetShaderResources(0u, 1u, &m_HeightmapSRV);
	DeviceContext->VSSetSamplers(0u, 1u, Graphics::GetSingletonPtr()->GetSamplerState().GetAddressOf());

	DeviceContext->HSSetShader(m_HullShader.Get(), nullptr, 0u);
	DeviceContext->HSSetConstantBuffers(0u, 1u, m_HullCBuffer.GetAddressOf());

	DeviceContext->DSSetShader(m_DomainShader.Get(), nullptr, 0u);
	DeviceContext->DSSetConstantBuffers(0u, 1u, m_pLandscape->m_CameraCBuffer.GetAddressOf());
	DeviceContext->DSSetConstantBuffers(1u, 1u, m_pLandscape->m_LandscapeInfoCBuffer.GetAddressOf());
	DeviceContext->DSSetShaderResources(0u, 1u, &m_HeightmapSRV);
	DeviceContext->DSSetSamplers(0u, 1u, Graphics::GetSingletonPtr()->GetSamplerState().GetAddressOf());

	DeviceContext->GSSetShader(m_GeometryShader.Get(), nullptr, 0u);
	DeviceContext->GSSetConstantBuffers(0u, 1u, m_pLandscape->m_CullingCBuffer.GetAddressOf());

	DeviceContext->PSSetShader(m_PixelShader.Get(), nullptr, 0u);
	DeviceContext->PSSetConstantBuffers(1u, 1u, m_pLandscape->m_LandscapeInfoCBuffer.GetAddressOf());
	DeviceContext->PSSetShaderResources(0u, 1u, &m_HeightmapSRV);
	DeviceContext->PSSetSamplers(0u, 1u, Graphics::GetSingletonPtr()->GetSamplerState().GetAddressOf());

	DeviceContext->DrawIndexedInstanced(4u, m_pLandscape->m_NumChunks, 0u, 0u, 0u);

	DeviceContext->HSSetShader(nullptr, nullptr, 0u);
	DeviceContext->DSSetShader(nullptr, nullptr, 0u);
	DeviceContext->GSSetShader(nullptr, nullptr, 0u);
}

void TessellatedPlane::Shutdown()
{
	m_InputLayout.Reset();
	m_IndexBuffer.Reset();
	m_VertexShader.Reset();
	m_HullShader.Reset();
	m_DomainShader.Reset();
	m_GeometryShader.Reset();
	m_PixelShader.Reset();
	m_VertexBuffer.Reset();
	m_HullCBuffer.Reset();

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
	Microsoft::WRL::ComPtr<ID3D10Blob> ErrorMessage;
	Microsoft::WRL::ComPtr<ID3D10Blob> vsBuffer;
	Microsoft::WRL::ComPtr<ID3D10Blob> hsBuffer;
	Microsoft::WRL::ComPtr<ID3D10Blob> dsBuffer;
	Microsoft::WRL::ComPtr<ID3D10Blob> gsBuffer;
	Microsoft::WRL::ComPtr<ID3D10Blob> psBuffer;
	wchar_t vsFilename[32];
	wchar_t hsFilename[32];
	wchar_t dsFilename[32];
	wchar_t gsFilename[32];
	wchar_t psFilename[32];

	HWND hWnd = Application::GetSingletonPtr()->GetWindowHandle();
	ID3D11Device* Device = Graphics::GetSingletonPtr()->GetDevice();

	wcscpy_s(vsFilename, 32, L"Shaders/TessellatedPlaneVS.hlsl");
	wcscpy_s(hsFilename, 32, L"Shaders/TessellatedPlaneHS.hlsl");
	wcscpy_s(dsFilename, 32, L"Shaders/TessellatedPlaneDS.hlsl");
	wcscpy_s(gsFilename, 32, L"Shaders/TessellatedPlaneGS.hlsl");
	wcscpy_s(psFilename, 32, L"Shaders/TessellatedPlanePS.hlsl");

	UINT CompileFlags = D3D10_SHADER_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
	hResult = D3DCompileFromFile(vsFilename, NULL, NULL, "main", "vs_5_0", CompileFlags, 0, &vsBuffer, &ErrorMessage);
	if (FAILED(hResult))
	{
		if (ErrorMessage.Get())
		{
			Shader::OutputShaderErrorMessage(ErrorMessage.Get(), hWnd, vsFilename);
		}
		else
		{
			MessageBox(hWnd, vsFilename, L"Missing shader file!", MB_OK);
		}

		return false;
	}

	hResult = D3DCompileFromFile(hsFilename, NULL, NULL, "main", "hs_5_0", CompileFlags, 0, &hsBuffer, &ErrorMessage);
	if (FAILED(hResult))
	{
		if (ErrorMessage.Get())
		{
			Shader::OutputShaderErrorMessage(ErrorMessage.Get(), hWnd, hsFilename);
		}
		else
		{
			MessageBox(hWnd, hsFilename, L"Missing shader file!", MB_OK);
		}

		return false;
	}

	hResult = D3DCompileFromFile(dsFilename, NULL, NULL, "main", "ds_5_0", CompileFlags, 0, &dsBuffer, &ErrorMessage);
	if (FAILED(hResult))
	{
		if (ErrorMessage.Get())
		{
			Shader::OutputShaderErrorMessage(ErrorMessage.Get(), hWnd, dsFilename);
		}
		else
		{
			MessageBox(hWnd, dsFilename, L"Missing shader file!", MB_OK);
		}

		return false;
	}

	hResult = D3DCompileFromFile(gsFilename, NULL, NULL, "main", "gs_5_0", CompileFlags, 0, &gsBuffer, &ErrorMessage);
	if (FAILED(hResult))
	{
		if (ErrorMessage.Get())
		{
			Shader::OutputShaderErrorMessage(ErrorMessage.Get(), hWnd, gsFilename);
		}
		else
		{
			MessageBox(hWnd, gsFilename, L"Missing shader file!", MB_OK);
		}

		return false;
	}

	hResult = D3DCompileFromFile(psFilename, NULL, NULL, "main", "ps_5_0", CompileFlags, 0, &psBuffer, &ErrorMessage);
	if (FAILED(hResult))
	{
		if (ErrorMessage.Get())
		{
			Shader::OutputShaderErrorMessage(ErrorMessage.Get(), hWnd, psFilename);
		}
		else
		{
			MessageBox(hWnd, psFilename, L"Missing shader file!", MB_OK);
		}

		return false;
	}

	HFALSE_IF_FAILED(Device->CreateVertexShader(vsBuffer->GetBufferPointer(), vsBuffer->GetBufferSize(), NULL, &m_VertexShader));
	HFALSE_IF_FAILED(Device->CreateHullShader(hsBuffer->GetBufferPointer(), hsBuffer->GetBufferSize(), NULL, &m_HullShader));
	HFALSE_IF_FAILED(Device->CreateDomainShader(dsBuffer->GetBufferPointer(), dsBuffer->GetBufferSize(), NULL, &m_DomainShader));
	HFALSE_IF_FAILED(Device->CreateGeometryShader(gsBuffer->GetBufferPointer(), gsBuffer->GetBufferSize(), NULL, &m_GeometryShader));
	HFALSE_IF_FAILED(Device->CreatePixelShader(psBuffer->GetBufferPointer(), psBuffer->GetBufferSize(), NULL, &m_PixelShader));

	NAME_D3D_RESOURCE(m_VertexShader, "Tessellated plane vertex shader");
	NAME_D3D_RESOURCE(m_HullShader, "Tessellated plane hull shader");
	NAME_D3D_RESOURCE(m_DomainShader, "Tessellated plane domain shader");
	NAME_D3D_RESOURCE(m_GeometryShader, "Tessellated plane geometry shader");
	NAME_D3D_RESOURCE(m_PixelShader, "Tessellated plane pixel shader");
	
	D3D11_INPUT_ELEMENT_DESC LayoutDesc[1] = {};
	LayoutDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	LayoutDesc[0].SemanticName = "POSITION";
	LayoutDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	HFALSE_IF_FAILED(Device->CreateInputLayout(LayoutDesc, _countof(LayoutDesc), vsBuffer->GetBufferPointer(), vsBuffer->GetBufferSize(), &m_InputLayout));
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
	
	DirectX::XMFLOAT3 CameraPos = Application::GetSingletonPtr()->GetActiveCamera()->GetPosition();
	
	ASSERT_NOT_FAILED(DeviceContext->Map(m_HullCBuffer.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource));
	HullCBufferPtr = (HullCBuffer*)MappedResource.pData;
	HullCBufferPtr->CameraPos = CameraPos;
	HullCBufferPtr->TessellationScale = m_TessellationScale;
	DeviceContext->Unmap(m_HullCBuffer.Get(), 0u);
}
