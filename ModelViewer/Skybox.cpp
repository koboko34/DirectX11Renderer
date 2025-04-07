#include "d3dcompiler.h"

#include "Skybox.h"
#include "Graphics.h"
#include "MyMacros.h"
#include "ResourceManager.h"
#include "Application.h"
#include "Camera.h"

struct Vertex
{
	DirectX::XMFLOAT3 Position;
};

const Vertex CubeVertices[] =
{
	{{-0.5f,  0.5f, -0.5f}},
	{{ 0.5f,  0.5f, -0.5f}},
	{{ 0.5f, -0.5f, -0.5f}},
	{{-0.5f, -0.5f, -0.5f}},
	{{-0.5f,  0.5f,  0.5f}},
	{{ 0.5f,  0.5f,  0.5f}},
	{{ 0.5f, -0.5f,  0.5f}},
	{{-0.5f, -0.5f,  0.5f}},
};

const UINT CubeIndices[] =
{
	2, 1, 0, 0, 3, 2, // -Z
	1, 5, 4, 4, 0, 1, // +Y
	5, 6, 7, 7, 4, 5, // +Z
	6, 2, 3, 3, 7, 6, // -Y
	6, 5, 1, 1, 2, 6, // +X
	3, 0, 4, 4, 7, 3  // -X
};

bool Skybox::Init()
{
	HRESULT hResult;
	ID3D11Texture2D* pTexture;
	D3D11_TEXTURE2D_DESC CubeDesc = {};
	ID3D11Device* Device = Graphics::GetSingletonPtr()->GetDevice();
	ID3D11DeviceContext* DeviceContext = Graphics::GetSingletonPtr()->GetDeviceContext();

	LoadTextures();
	m_Textures[0]->GetDesc(&CubeDesc);
	CubeDesc.ArraySize = 6u;
	CubeDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

	std::vector<std::vector<BYTE>> TextureData(6);
	D3D11_SUBRESOURCE_DATA Data[6] = {};
	for (int i = 0; i < 6; i++)
	{
		ID3D11Texture2D* OriginalTex = m_Textures[i];
		D3D11_TEXTURE2D_DESC Desc;
		OriginalTex->GetDesc(&Desc);

		Desc.Usage = D3D11_USAGE_STAGING;
		Desc.BindFlags = 0;
		Desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		Desc.MiscFlags = 0;

		ID3D11Texture2D* StagingTex = nullptr;
		HFALSE_IF_FAILED(Device->CreateTexture2D(&Desc, nullptr, &StagingTex));
		DeviceContext->CopyResource(StagingTex, OriginalTex);

		D3D11_MAPPED_SUBRESOURCE Mapped;
		DeviceContext->Map(StagingTex, 0, D3D11_MAP_READ, 0, &Mapped);

		size_t DataSize = Mapped.RowPitch * Desc.Height;
		TextureData[i].resize(DataSize);
		memcpy(TextureData[i].data(), Mapped.pData, DataSize);

		DeviceContext->Unmap(StagingTex, 0);
		StagingTex->Release();

		Data[i].pSysMem = TextureData[i].data();
		Data[i].SysMemPitch = Mapped.RowPitch;
	}

	for (const std::string& Filename : m_FileNames)
	{
		ResourceManager::GetSingletonPtr()->UnloadTexture(m_TexturesDir + Filename);
	}
	m_Textures.clear();

	HFALSE_IF_FAILED(Device->CreateTexture2D(&CubeDesc, Data, &pTexture));

	D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
	SRVDesc.Format = CubeDesc.Format;
	SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	SRVDesc.Texture2D.MostDetailedMip = 0u;
	SRVDesc.Texture2D.MipLevels = 1u;
	HFALSE_IF_FAILED(Device->CreateShaderResourceView(pTexture, &SRVDesc, &m_SRV));

	bool Result;
	FALSE_IF_FAILED(CreateBuffers());

	Microsoft::WRL::ComPtr<ID3D10Blob> ErrorMessage;
	Microsoft::WRL::ComPtr<ID3D10Blob> vsBuffer;
	Microsoft::WRL::ComPtr<ID3D10Blob> psBuffer;

	UINT CompileFlags = D3D10_SHADER_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
	HFALSE_IF_FAILED(D3DCompileFromFile(L"Shaders/SkyboxVS.hlsl", NULL, NULL, "main", "vs_5_0", CompileFlags, 0, &vsBuffer, &ErrorMessage));
	HFALSE_IF_FAILED(D3DCompileFromFile(L"Shaders/SkyboxPS.hlsl", NULL, NULL, "main", "ps_5_0", CompileFlags, 0, &psBuffer, &ErrorMessage));

	HFALSE_IF_FAILED(Device->CreateVertexShader(vsBuffer->GetBufferPointer(), vsBuffer->GetBufferSize(), NULL, &m_VertexShader));
	HFALSE_IF_FAILED(Device->CreatePixelShader(psBuffer->GetBufferPointer(), psBuffer->GetBufferSize(), NULL, &m_PixelShader));

	D3D11_INPUT_ELEMENT_DESC Layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	HFALSE_IF_FAILED(Device->CreateInputLayout(Layout, 1u, vsBuffer->GetBufferPointer(), vsBuffer->GetBufferSize(), &m_InputLayout));
	
	return true;
}

void Skybox::Render()
{
	UINT Strides[] = { sizeof(Vertex) };
	UINT Offsets[] = { 0u };

	ID3D11DeviceContext* DeviceContext = Graphics::GetSingletonPtr()->GetDeviceContext();
	DeviceContext->IASetInputLayout(m_InputLayout.Get());
	DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	DeviceContext->IASetVertexBuffers(0u, 1u, m_VertexBuffer.GetAddressOf(), Strides, Offsets);
	DeviceContext->IASetIndexBuffer(m_IndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0u);
	DeviceContext->VSSetShader(m_VertexShader.Get(), nullptr, 0u);
	DeviceContext->VSSetConstantBuffers(0u, 1u, m_ConstantBuffer.GetAddressOf());
	DeviceContext->PSSetShader(m_PixelShader.Get(), nullptr, 0u);
	DeviceContext->PSSetShaderResources(0u, 1u, m_SRV.GetAddressOf());

	Graphics::GetSingletonPtr()->DisableDepthWriteAlwaysPass();
	Graphics::GetSingletonPtr()->SetRasterStateBackFaceCull(true);

	HRESULT hResult;
	D3D11_MAPPED_SUBRESOURCE MappedResource;

	struct BufferData
	{
		DirectX::XMMATRIX Matrix;
	};
	BufferData* DataPtr;

	DirectX::XMMATRIX View, Proj, ViewProj;
	Application::GetSingletonPtr()->GetCamera()->GetViewMatrix(View);
	Graphics::GetSingletonPtr()->GetProjectionMatrix(Proj);

	View.r[3] = DirectX::XMVectorSet(0.f, 0.f, 0.f, 1.f); // removes translation from the view matrix
	ViewProj = View * Proj;
	
	// remember to transpose from row major before sending to shaders
	ViewProj = DirectX::XMMatrixTranspose(ViewProj);

	ASSERT_NOT_FAILED(DeviceContext->Map(m_ConstantBuffer.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource));
	DataPtr = (BufferData*)MappedResource.pData;
	DataPtr->Matrix = ViewProj;
	DeviceContext->Unmap(m_ConstantBuffer.Get(), 0u);

	DeviceContext->DrawIndexed(sizeof(CubeIndices) / sizeof(UINT), 0u, 0);

	ID3D11ShaderResourceView* NullSRVs[] = { nullptr };
	DeviceContext->PSSetShaderResources(0u, 1u, NullSRVs);
}

void Skybox::Shutdown()
{
}

bool Skybox::LoadTextures()
{
	for (const std::string& Filename : m_FileNames)
	{
		ID3D11ShaderResourceView* SRV = ResourceManager::GetSingletonPtr()->LoadTexture(m_TexturesDir + Filename);
		if (!SRV)
		{
			return false;
		}
		
		ID3D11Resource* Resource = nullptr;
		SRV->GetResource(&Resource);

		ID3D11Texture2D* Texture = nullptr;
		HRESULT hResult;
		
		HFALSE_IF_FAILED(Resource->QueryInterface(__uuidof(ID3D11Texture2D), (void**)&Texture));
		m_Textures.push_back(Texture);
	}
	return true;
}

bool Skybox::CreateBuffers()
{
	HRESULT hResult;

	D3D11_BUFFER_DESC Desc = {};
	Desc.Usage = D3D11_USAGE_IMMUTABLE;
	Desc.ByteWidth = sizeof(CubeVertices);
	Desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA Data = {};
	Data.pSysMem = CubeVertices;

	HFALSE_IF_FAILED(Graphics::GetSingletonPtr()->GetDevice()->CreateBuffer(&Desc, &Data, &m_VertexBuffer));

	Desc.Usage = D3D11_USAGE_IMMUTABLE;
	Desc.ByteWidth = sizeof(CubeIndices);
	Desc.BindFlags = D3D11_BIND_INDEX_BUFFER;

	Data.pSysMem = CubeIndices;

	HFALSE_IF_FAILED(Graphics::GetSingletonPtr()->GetDevice()->CreateBuffer(&Desc, &Data, &m_IndexBuffer));

	Desc.Usage = D3D11_USAGE_DYNAMIC;
	Desc.ByteWidth = sizeof(DirectX::XMMATRIX);
	Desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	Desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	HFALSE_IF_FAILED(Graphics::GetSingletonPtr()->GetDevice()->CreateBuffer(&Desc, nullptr, &m_ConstantBuffer));
	
	return true;
}
