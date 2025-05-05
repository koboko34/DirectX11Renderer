#include "d3dcompiler.h"

#include "BoxRenderer.h"
#include "Graphics.h"
#include "MyMacros.h"
#include "Camera.h"
#include "Application.h"
#include "AABB.h"
#include "ResourceManager.h"

const UINT BoxIndices[12][2] = {
	{0, 1}, {1, 3}, {3, 2}, {2, 0},
	{4, 5}, {5, 7}, {7, 6}, {6, 4},
	{0, 4}, {1, 5}, {2, 6}, {3, 7}
};

BoxRenderer::~BoxRenderer()
{
	Shutdown();
}

bool BoxRenderer::Init()
{
	bool Result;
	FALSE_IF_FAILED(CreateShaders());
	FALSE_IF_FAILED(CreateBuffers());

	m_BoxCorners.resize(8);

	return true;
}

void BoxRenderer::Shutdown()
{
	ResourceManager::GetSingletonPtr()->UnloadShader<ID3D11VertexShader>(m_vsFilename, "main");
	ResourceManager::GetSingletonPtr()->UnloadShader<ID3D11PixelShader>(m_psFilename, "main");
}

void BoxRenderer::Render()
{
	Graphics::GetSingletonPtr()->DisableBlending();
	Graphics::GetSingletonPtr()->EnableDepthWrite();

	ID3D11DeviceContext* DeviceContext = Graphics::GetSingletonPtr()->GetDeviceContext();
	UINT Strides[] = { sizeof(DirectX::XMFLOAT3), 0u };
	UINT Offsets[] = { 0u };
	ID3D11Buffer* Buffers[] = { m_VertexBuffer.Get() };

	UpdateBuffers();

	DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	DeviceContext->IASetInputLayout(m_InputLayout.Get());
	DeviceContext->IASetVertexBuffers(0u, 1u, Buffers, Strides, Offsets);
	DeviceContext->IASetIndexBuffer(m_IndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0u);

	DeviceContext->VSSetShader(m_VertexShader, nullptr, 0u);
	DeviceContext->VSSetConstantBuffers(0u, 1u, m_VertexCBuffer.GetAddressOf());

	DeviceContext->PSSetShader(m_PixelShader, nullptr, 0u);

	Graphics::GetSingletonPtr()->DisableDepthWrite();
	Graphics::GetSingletonPtr()->DisableBlending();

	DeviceContext->DrawIndexed(24u, 0u, 0u);
	Application::GetSingletonPtr()->GetRenderStatsRef().DrawCalls++;
}

void BoxRenderer::RenderBox(const AABB& BBox, const DirectX::XMMATRIX& Transform)
{
	LoadBoxCorners(BBox, Transform);
	Render();
}

void BoxRenderer::RenderFrustum(const std::shared_ptr<Camera>& pCamera)
{
	LoadFrustumCorners(pCamera);
	Render();
}

bool BoxRenderer::CreateShaders()
{
	HRESULT hResult;
	Microsoft::WRL::ComPtr<ID3D10Blob> vsBuffer;

	m_vsFilename = "Shaders/BoxRendererVS.hlsl";
	m_psFilename = "Shaders/BoxRendererPS.hlsl";

	m_VertexShader = ResourceManager::GetSingletonPtr()->LoadShader<ID3D11VertexShader>(m_vsFilename, "main", vsBuffer);
	m_PixelShader = ResourceManager::GetSingletonPtr()->LoadShader<ID3D11PixelShader>(m_psFilename, "main");

	D3D11_INPUT_ELEMENT_DESC LayoutDesc[1] = {};
	LayoutDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	LayoutDesc[0].SemanticName = "POSITION";
	LayoutDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	HFALSE_IF_FAILED(Graphics::GetSingletonPtr()->GetDevice()->CreateInputLayout(LayoutDesc, _countof(LayoutDesc), vsBuffer->GetBufferPointer(), vsBuffer->GetBufferSize(), &m_InputLayout));
	NAME_D3D_RESOURCE(m_InputLayout, "Box renderer input layout");

	return true;
}

bool BoxRenderer::CreateBuffers()
{
	HRESULT hResult;

	D3D11_BUFFER_DESC Desc = {};
	Desc.Usage = D3D11_USAGE_IMMUTABLE;
	Desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	Desc.ByteWidth = sizeof(BoxIndices);

	D3D11_SUBRESOURCE_DATA Data = {};
	Data.pSysMem = BoxIndices;

	HFALSE_IF_FAILED(Graphics::GetSingletonPtr()->GetDevice()->CreateBuffer(&Desc, &Data, &m_IndexBuffer));
	NAME_D3D_RESOURCE(m_IndexBuffer, "Box renderer index buffer");

	Desc.Usage = D3D11_USAGE_DYNAMIC;
	Desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	Desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	Desc.ByteWidth = sizeof(DirectX::XMFLOAT3) * 8;

	HFALSE_IF_FAILED(Graphics::GetSingletonPtr()->GetDevice()->CreateBuffer(&Desc, nullptr, &m_VertexBuffer));
	NAME_D3D_RESOURCE(m_VertexBuffer, "Box renderer vertex buffer");

	Desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	Desc.ByteWidth = sizeof(DirectX::XMMATRIX);

	HFALSE_IF_FAILED(Graphics::GetSingletonPtr()->GetDevice()->CreateBuffer(&Desc, nullptr, &m_VertexCBuffer));
	NAME_D3D_RESOURCE(m_VertexCBuffer, "Box renderer vertex constant buffer");

	return true;
}

void BoxRenderer::UpdateBuffers()
{
	HRESULT hResult;
	D3D11_MAPPED_SUBRESOURCE MappedResource;
	ID3D11DeviceContext* DeviceContext = Graphics::GetSingletonPtr()->GetDeviceContext();

	ASSERT_NOT_FAILED(DeviceContext->Map(m_VertexBuffer.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource));
	memcpy(MappedResource.pData, m_BoxCorners.data(), sizeof(DirectX::XMFLOAT3) * 8);
	DeviceContext->Unmap(m_VertexBuffer.Get(), 0u);

	ASSERT_NOT_FAILED(DeviceContext->Map(m_VertexCBuffer.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource));
	DirectX::XMMATRIX ViewProj = DirectX::XMMatrixTranspose(Application::GetSingletonPtr()->GetActiveCamera()->GetViewProjMatrix());
	memcpy(MappedResource.pData, &ViewProj, sizeof(DirectX::XMMATRIX));
	DeviceContext->Unmap(m_VertexCBuffer.Get(), 0u);
}

void BoxRenderer::LoadBoxCorners(const AABB& BBox, const DirectX::XMMATRIX& Transform)
{
	for (size_t i = 0; i < 8; i++)
	{
		DirectX::XMVECTOR Corner = DirectX::XMVectorSet(BBox.Corners[i].x, BBox.Corners[i].y, BBox.Corners[i].z, 1.f);
		DirectX::XMVECTOR WorldCorner = DirectX::XMVector4Transform(Corner, Transform);

		DirectX::XMStoreFloat3(&m_BoxCorners[i], WorldCorner);
	}
}

void BoxRenderer::LoadFrustumCorners(const std::shared_ptr<Camera>& pCamera)
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

				DirectX::XMStoreFloat3(&m_BoxCorners[i++], WorldCorner);
			}
		}
	}
}
