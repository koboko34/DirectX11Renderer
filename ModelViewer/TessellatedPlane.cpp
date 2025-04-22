#include "d3dcompiler.h"

#include "ImGui/imgui.h"

#include "TessellatedPlane.h"
#include "MyMacros.h"
#include "Graphics.h"
#include "Application.h"
#include "Shader.h"
#include "Camera.h"

struct Vertex
{
	DirectX::XMFLOAT3 Position;
};

const Vertex PlaneVertices[] =
{
	{{-0.5f, 0.f, -0.5f}},
	{{-0.5f, 0.f,  0.5f}},
	{{ 0.5f, 0.f, -0.5f}},
	{{ 0.5f, 0.f,  0.5f}},
};

TessellatedPlane::TessellatedPlane()
{
	SetName("Tessellated Plane");
	m_bShouldRender = true;
}

bool TessellatedPlane::Init()
{
	bool Result;
	FALSE_IF_FAILED(CreateShaders());
	FALSE_IF_FAILED(CreateBuffers());
    
    return true;
}

void TessellatedPlane::Render()
{
	ID3D11DeviceContext* DeviceContext = Graphics::GetSingletonPtr()->GetDeviceContext();
	UINT Strides[] = { sizeof(Vertex) };
	UINT Offsets[] = { 0u };

	DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);
	DeviceContext->IASetVertexBuffers(0u, 1u, m_VertexBuffer.GetAddressOf(), Strides, Offsets);

	DeviceContext->VSSetShader(m_VertexShader.Get(), nullptr, 0u);
	DeviceContext->VSSetConstantBuffers(0u, 1u, m_VertexConstantBuffer.GetAddressOf());

	DeviceContext->HSSetShader(m_HullShader.Get(), nullptr, 0u);
	DeviceContext->HSSetConstantBuffers(0u, 1u, m_HullConstantBuffer.GetAddressOf());

	DeviceContext->DSSetShader(m_DomainShader.Get(), nullptr, 0u);
	DeviceContext->DSSetConstantBuffers(0u, 1u, m_DomainConstantBuffer.GetAddressOf());

	DeviceContext->PSSetShader(m_PixelShader.Get(), nullptr, 0u);

	UpdateBuffers();

	DeviceContext->Draw(4u, 0u);

	DeviceContext->HSSetShader(nullptr, nullptr, 0u);
	DeviceContext->DSSetShader(nullptr, nullptr, 0u);
}

void TessellatedPlane::Shutdown()
{
	m_InputLayout.Reset();
	m_VertexShader.Reset();
	m_HullShader.Reset();
	m_DomainShader.Reset();
	m_PixelShader.Reset();
	m_VertexBuffer.Reset();
	m_VertexConstantBuffer.Reset();
	m_HullConstantBuffer.Reset();
	m_DomainConstantBuffer.Reset();
}

void TessellatedPlane::RenderControls()
{
	ImGui::Text("Transform");

	float Scale = m_Transform.Scale.x;
	if (ImGui::DragFloat("ScaleXZ", reinterpret_cast<float*>(&Scale), 0.1f))
	{
		SetScale(Scale, Scale, Scale);
	}

	ImGui::Checkbox("Should Render?", &m_bShouldRender);
}

bool TessellatedPlane::CreateShaders()
{
	HRESULT hResult;
	Microsoft::WRL::ComPtr<ID3D10Blob> ErrorMessage;
	Microsoft::WRL::ComPtr<ID3D10Blob> vsBuffer;
	Microsoft::WRL::ComPtr<ID3D10Blob> hsBuffer;
	Microsoft::WRL::ComPtr<ID3D10Blob> dsBuffer;
	Microsoft::WRL::ComPtr<ID3D10Blob> psBuffer;
	wchar_t vsFilename[32];
	wchar_t hsFilename[32];
	wchar_t dsFilename[32];
	wchar_t psFilename[32];

	HWND hWnd = Application::GetSingletonPtr()->GetWindowHandle();
	ID3D11Device* Device = Graphics::GetSingletonPtr()->GetDevice();

	wcscpy_s(vsFilename, 32, L"Shaders/TessellatedPlaneVS.hlsl");
	wcscpy_s(hsFilename, 32, L"Shaders/TessellatedPlaneHS.hlsl");
	wcscpy_s(dsFilename, 32, L"Shaders/TessellatedPlaneDS.hlsl");
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
	HFALSE_IF_FAILED(Device->CreatePixelShader(psBuffer->GetBufferPointer(), psBuffer->GetBufferSize(), NULL, &m_PixelShader));
	
	D3D11_INPUT_ELEMENT_DESC LayoutDesc[1] = {};
	LayoutDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	LayoutDesc[0].SemanticName = "POSITION";
	LayoutDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	HFALSE_IF_FAILED(Device->CreateInputLayout(LayoutDesc, _countof(LayoutDesc), vsBuffer->GetBufferPointer(), vsBuffer->GetBufferSize(), &m_InputLayout));
	
	return true;
}

bool TessellatedPlane::CreateBuffers()
{
	HRESULT hResult;

	D3D11_BUFFER_DESC Desc = {};
	Desc.Usage = D3D11_USAGE_IMMUTABLE;
	Desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	Desc.ByteWidth = sizeof(PlaneVertices);

	D3D11_SUBRESOURCE_DATA Data = {};
	Data.pSysMem = PlaneVertices;

	HFALSE_IF_FAILED(Graphics::GetSingletonPtr()->GetDevice()->CreateBuffer(&Desc, &Data, &m_VertexBuffer));

	Desc.Usage = D3D11_USAGE_DYNAMIC;
	Desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	Desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	Desc.ByteWidth = sizeof(TransformBuffer);

	HFALSE_IF_FAILED(Graphics::GetSingletonPtr()->GetDevice()->CreateBuffer(&Desc, nullptr, &m_VertexConstantBuffer));

	Desc.ByteWidth = sizeof(CameraBuffer);

	HFALSE_IF_FAILED(Graphics::GetSingletonPtr()->GetDevice()->CreateBuffer(&Desc, nullptr, &m_HullConstantBuffer));

	Desc.ByteWidth = sizeof(MatrixBuffer);

	HFALSE_IF_FAILED(Graphics::GetSingletonPtr()->GetDevice()->CreateBuffer(&Desc, nullptr, &m_DomainConstantBuffer));
	
	return true;
}

void TessellatedPlane::UpdateBuffers()
{	
	HRESULT hResult;
	D3D11_MAPPED_SUBRESOURCE MappedResource;
	TransformBuffer* TransformDataPtr;
	CameraBuffer* CameraDataPtr;
	MatrixBuffer* ViewProjPtr;
	ID3D11DeviceContext* DeviceContext = Graphics::GetSingletonPtr()->GetDeviceContext();

	ASSERT_NOT_FAILED(DeviceContext->Map(m_VertexConstantBuffer.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource));
	TransformDataPtr = (TransformBuffer*)MappedResource.pData;
	TransformDataPtr->Transform = DirectX::XMMatrixTranspose(DirectX::XMMatrixIdentity() * DirectX::XMMatrixScaling(GetScale().x, GetScale().y, GetScale().z));
	DeviceContext->Unmap(m_VertexConstantBuffer.Get(), 0u);
	
	Camera* pCamera = Application::GetSingletonPtr()->GetCamera();
	DirectX::XMFLOAT3 CameraPos = pCamera->GetPosition();
	DirectX::XMMATRIX View, Proj;
	pCamera->GetViewMatrix(View);
	Graphics::GetSingletonPtr()->GetProjectionMatrix(Proj);
	
	ASSERT_NOT_FAILED(DeviceContext->Map(m_HullConstantBuffer.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource));
	CameraDataPtr = (CameraBuffer*)MappedResource.pData;
	CameraDataPtr->CameraPos = CameraPos;
	CameraDataPtr->Padding = 0.f;
	DeviceContext->Unmap(m_HullConstantBuffer.Get(), 0u);

	ASSERT_NOT_FAILED(DeviceContext->Map(m_DomainConstantBuffer.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource));
	ViewProjPtr = (MatrixBuffer*)MappedResource.pData;
	ViewProjPtr->ViewProj = DirectX::XMMatrixTranspose(View * Proj);
	DeviceContext->Unmap(m_DomainConstantBuffer.Get(), 0u);
}
