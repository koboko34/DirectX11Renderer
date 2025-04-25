#include "d3dcompiler.h"

#include "FrustumRenderer.h"
#include "Graphics.h"
#include "MyMacros.h"
#include "Camera.h"
#include "Application.h"
#include "Shader.h"

const UINT FrustumIndices[12][2] = {
	{0, 1}, {1, 3}, {3, 2}, {2, 0},
	{4, 5}, {5, 7}, {7, 6}, {6, 4},
	{0, 4}, {1, 5}, {2, 6}, {3, 7}
};

bool FrustumRenderer::Init()
{
	bool Result;
	FALSE_IF_FAILED(CreateShaders());
	FALSE_IF_FAILED(CreateBuffers());

	m_FrustumCorners.resize(8);
	
	return true;
}

void FrustumRenderer::RenderFrustum(const std::shared_ptr<Camera>& pCamera)
{
	ID3D11DeviceContext* DeviceContext = Graphics::GetSingletonPtr()->GetDeviceContext();
	UINT Strides[] = { sizeof(DirectX::XMFLOAT3), 0u };
	UINT Offsets[] = { 0u };
	ID3D11Buffer* Buffers[] = { m_VertexBuffer.Get() };

	LoadFrustumCorners(pCamera);
	UpdateBuffers(pCamera);

	DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	DeviceContext->IASetInputLayout(m_InputLayout.Get());
	DeviceContext->IASetVertexBuffers(0u, 1u, Buffers, Strides, Offsets);
	DeviceContext->IASetIndexBuffer(m_IndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0u);

	DeviceContext->VSSetShader(m_VertexShader.Get(), nullptr, 0u);
	DeviceContext->VSSetConstantBuffers(0u, 1u, m_VertexCBuffer.GetAddressOf());

	DeviceContext->PSSetShader(m_PixelShader.Get(), nullptr, 0u);

	Graphics::GetSingletonPtr()->DisableDepthWrite();
	
	DeviceContext->DrawIndexed(24u, 0u, 0u);
}

bool FrustumRenderer::CreateShaders()
{
	HRESULT hResult;
	Microsoft::WRL::ComPtr<ID3D10Blob> ErrorMessage;
	Microsoft::WRL::ComPtr<ID3D10Blob> vsBuffer;
	Microsoft::WRL::ComPtr<ID3D10Blob> psBuffer;
	wchar_t vsFilename[32];
	wchar_t psFilename[32];

	HWND hWnd = Application::GetSingletonPtr()->GetWindowHandle();
	ID3D11Device* Device = Graphics::GetSingletonPtr()->GetDevice();

	wcscpy_s(vsFilename, 32, L"Shaders/FrustumRendererVS.hlsl");
	wcscpy_s(psFilename, 32, L"Shaders/FrustumRendererPS.hlsl");

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
	HFALSE_IF_FAILED(Device->CreatePixelShader(psBuffer->GetBufferPointer(), psBuffer->GetBufferSize(), NULL, &m_PixelShader));

	NAME_D3D_RESOURCE(m_VertexShader, "Frustum renderer vertex shader");
	NAME_D3D_RESOURCE(m_PixelShader, "Frustum renderer pixel shader");

	D3D11_INPUT_ELEMENT_DESC LayoutDesc[1] = {};
	LayoutDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	LayoutDesc[0].SemanticName = "POSITION";
	LayoutDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	HFALSE_IF_FAILED(Device->CreateInputLayout(LayoutDesc, _countof(LayoutDesc), vsBuffer->GetBufferPointer(), vsBuffer->GetBufferSize(), &m_InputLayout));
	NAME_D3D_RESOURCE(m_InputLayout, "Frustum renderer input layout");

	return true;
}

bool FrustumRenderer::CreateBuffers()
{
	HRESULT hResult;

	D3D11_BUFFER_DESC Desc = {};
	Desc.Usage = D3D11_USAGE_IMMUTABLE;
	Desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	Desc.ByteWidth = sizeof(FrustumIndices);

	D3D11_SUBRESOURCE_DATA Data = {};
	Data.pSysMem = FrustumIndices;

	HFALSE_IF_FAILED(Graphics::GetSingletonPtr()->GetDevice()->CreateBuffer(&Desc, &Data, &m_IndexBuffer));
	NAME_D3D_RESOURCE(m_IndexBuffer, "Frustum renderer index buffer");

	Desc.Usage = D3D11_USAGE_DYNAMIC;
	Desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	Desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	Desc.ByteWidth = sizeof(DirectX::XMFLOAT3) * 8;

	HFALSE_IF_FAILED(Graphics::GetSingletonPtr()->GetDevice()->CreateBuffer(&Desc, nullptr, &m_VertexBuffer));
	NAME_D3D_RESOURCE(m_VertexBuffer, "Frustum renderer vertex buffer");

	Desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	Desc.ByteWidth = sizeof(DirectX::XMMATRIX);

	HFALSE_IF_FAILED(Graphics::GetSingletonPtr()->GetDevice()->CreateBuffer(&Desc, nullptr, &m_VertexCBuffer));
	NAME_D3D_RESOURCE(m_VertexBuffer, "Frustum renderer vertex constant buffer");

	return true;
}

void FrustumRenderer::UpdateBuffers(const std::shared_ptr<Camera>& pCamera)
{
	HRESULT hResult;
	D3D11_MAPPED_SUBRESOURCE MappedResource;
	ID3D11DeviceContext* DeviceContext = Graphics::GetSingletonPtr()->GetDeviceContext();

	ASSERT_NOT_FAILED(DeviceContext->Map(m_VertexBuffer.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource));
	memcpy(MappedResource.pData, m_FrustumCorners.data(), sizeof(DirectX::XMFLOAT3) * 8);
	DeviceContext->Unmap(m_VertexBuffer.Get(), 0u);

	ASSERT_NOT_FAILED(DeviceContext->Map(m_VertexCBuffer.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource));
	DirectX::XMMATRIX ViewProj = DirectX::XMMatrixTranspose(Application::GetSingletonPtr()->GetActiveCamera()->GetViewProjMatrix());
	memcpy(MappedResource.pData, &ViewProj, sizeof(DirectX::XMMATRIX));
	DeviceContext->Unmap(m_VertexCBuffer.Get(), 0u);
}

void FrustumRenderer::LoadFrustumCorners(const std::shared_ptr<Camera>& pCamera)
{
	DirectX::XMMATRIX ViewProj = DirectX::XMMatrixMultiply(pCamera->GetViewMatrix(), pCamera->GetProjMatrix());
	DirectX::XMMATRIX InvViewProj = DirectX::XMMatrixInverse(nullptr, ViewProj);

	int i = 0;
	for (int z = 0; z <= 1; ++z)
	{
		float ndcZ = z * 2.0f - 1.0f;
		for (int y = 0; y <= 1; ++y)
		{
			float ndcY = y * 2.0f - 1.0f;
			for (int x = 0; x <= 1; ++x)
			{
				float ndcX = x * 2.0f - 1.0f;

				DirectX::XMVECTOR Corner = DirectX::XMVectorSet(ndcX, ndcY, ndcZ, 1.0f);
				DirectX::XMVECTOR WorldCorner = DirectX::XMVector4Transform(Corner, InvViewProj);
				WorldCorner = DirectX::XMVectorScale(WorldCorner, 1.0f / DirectX::XMVectorGetW(WorldCorner));

				DirectX::XMStoreFloat3(&m_FrustumCorners[i++], WorldCorner);
			}
		}
	}
}

