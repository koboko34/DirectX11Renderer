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

TessellatedPlane::TessellatedPlane(UINT ChunkDimension, float ChunkSize, float TessellationScale, float HeightDisplacement)
	: m_ChunkDimension(ChunkDimension), m_NumChunks(ChunkDimension * ChunkDimension), m_TessellationScale(TessellationScale), m_HeightDisplacement(HeightDisplacement)
{
	SetName("Tessellated Plane");
	SetScale(ChunkSize);
	m_bShouldRender = true;
	m_bVisualiseChunks = false;
	m_HeightmapSRV = nullptr;
	assert(m_NumChunks >= 0 && m_NumChunks <= MAX_PLANE_CHUNKS);
}

bool TessellatedPlane::Init(const std::string& HeightMapFilepath)
{
	bool Result;
	FALSE_IF_FAILED(CreateShaders());
	FALSE_IF_FAILED(CreateBuffers());
	
	m_HeightmapSRV = ResourceManager::GetSingletonPtr()->LoadTexture(HeightMapFilepath);
	assert(m_HeightmapSRV);
	
	return true;
}

void TessellatedPlane::Render()
{
	ID3D11DeviceContext* DeviceContext = Graphics::GetSingletonPtr()->GetDeviceContext();
	UINT Strides[] = { sizeof(PlaneVertex), 0u };
	UINT Offsets[] = { 0u, 0u };
	ID3D11Buffer* Buffers[2] = { m_VertexBuffer.Get(), nullptr };

	DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);
	DeviceContext->IASetInputLayout(m_InputLayout.Get());
	DeviceContext->IASetVertexBuffers(0u, 2u, Buffers, Strides, Offsets);
	DeviceContext->IASetIndexBuffer(m_IndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0u);

	DeviceContext->VSSetShader(m_VertexShader.Get(), nullptr, 0u);
	DeviceContext->VSSetConstantBuffers(0u, 1u, m_VertexCBuffer.GetAddressOf());
	DeviceContext->VSSetConstantBuffers(1u, 1u, m_PlaneInfoCBuffer.GetAddressOf());
	DeviceContext->VSSetShaderResources(0u, 1u, &m_HeightmapSRV);
	DeviceContext->VSSetSamplers(0u, 1u, Graphics::GetSingletonPtr()->GetSamplerState().GetAddressOf());

	DeviceContext->HSSetShader(m_HullShader.Get(), nullptr, 0u);
	DeviceContext->HSSetConstantBuffers(0u, 1u, m_HullCBuffer.GetAddressOf());

	DeviceContext->DSSetShader(m_DomainShader.Get(), nullptr, 0u);
	DeviceContext->DSSetConstantBuffers(0u, 1u, m_DomainCBuffer.GetAddressOf());
	DeviceContext->DSSetConstantBuffers(1u, 1u, m_PlaneInfoCBuffer.GetAddressOf());
	DeviceContext->DSSetShaderResources(0u, 1u, &m_HeightmapSRV);
	DeviceContext->DSSetSamplers(0u, 1u, Graphics::GetSingletonPtr()->GetSamplerState().GetAddressOf());

	DeviceContext->GSSetShader(m_GeometryShader.Get(), nullptr, 0u);
	DeviceContext->GSSetConstantBuffers(0u, 1u, m_GeometryCBuffer.GetAddressOf());

	DeviceContext->PSSetShader(m_PixelShader.Get(), nullptr, 0u);
	DeviceContext->PSSetConstantBuffers(1u, 1u, m_PlaneInfoCBuffer.GetAddressOf());
	DeviceContext->PSSetShaderResources(0u, 1u, &m_HeightmapSRV);
	DeviceContext->PSSetSamplers(0u, 1u, Graphics::GetSingletonPtr()->GetSamplerState().GetAddressOf());

	Graphics::GetSingletonPtr()->EnableDepthWrite();
	Graphics::GetSingletonPtr()->DisableBlending();

	UpdateBuffers();

	DeviceContext->DrawIndexedInstanced(4u, m_NumChunks, 0u, 0u, 0u);

	DeviceContext->HSSetShader(nullptr, nullptr, 0u);
	DeviceContext->DSSetShader(nullptr, nullptr, 0u);
	DeviceContext->GSSetShader(nullptr, nullptr, 0u);
}

void TessellatedPlane::Shutdown()
{
	m_InputLayout.Reset();
	m_VertexShader.Reset();
	m_HullShader.Reset();
	m_DomainShader.Reset();
	m_GeometryShader.Reset();
	m_PixelShader.Reset();
	m_VertexBuffer.Reset();
	m_VertexCBuffer.Reset();
	m_HullCBuffer.Reset();
	m_DomainCBuffer.Reset();
	m_GeometryCBuffer.Reset();
	m_PlaneInfoCBuffer.Reset();

	ResourceManager::GetSingletonPtr()->UnloadTexture("Textures/uk_heightmap.jpg");
	m_HeightmapSRV = nullptr;
}

void TessellatedPlane::RenderControls()
{
	ImGui::Text("Transform");

	float Scale = m_Transform.Scale.x;
	if (ImGui::DragFloat("ScaleXZ", &Scale, 0.1f))
	{
		SetScale(Scale, Scale, Scale);
	}

	ImGui::DragFloat("Tessellation Scale", &m_TessellationScale, 0.1f, 1.f, 256.f, "%.f", ImGuiSliderFlags_AlwaysClamp);
	ImGui::DragFloat("Height Displacement", &m_HeightDisplacement, 0.1f, 0.f, 256.f, "%.f", ImGuiSliderFlags_AlwaysClamp);

	ImGui::Checkbox("Should Render?", &m_bShouldRender);
	ImGui::Checkbox("Visualise Chunks?", &m_bVisualiseChunks);
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

	Desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	Desc.ByteWidth = sizeof(TransformCBuffer) * MAX_PLANE_CHUNKS;

	std::vector<DirectX::XMMATRIX> ChunkTransforms(MAX_PLANE_CHUNKS, DirectX::XMMatrixIdentity());
	GenerateChunkTransforms(ChunkTransforms);
	Data.pSysMem = ChunkTransforms.data();

	HFALSE_IF_FAILED(Graphics::GetSingletonPtr()->GetDevice()->CreateBuffer(&Desc, &Data, &m_VertexCBuffer));
	NAME_D3D_RESOURCE(m_VertexCBuffer, "Tessellated plane vertex constant buffer");

	Desc.Usage = D3D11_USAGE_DYNAMIC;
	Desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	Desc.ByteWidth = sizeof(CameraCBuffer);

	HFALSE_IF_FAILED(Graphics::GetSingletonPtr()->GetDevice()->CreateBuffer(&Desc, nullptr, &m_HullCBuffer));
	NAME_D3D_RESOURCE(m_HullCBuffer, "Tessellated plane hull constant buffer");

	Desc.ByteWidth = sizeof(DomainCBuffer);

	HFALSE_IF_FAILED(Graphics::GetSingletonPtr()->GetDevice()->CreateBuffer(&Desc, nullptr, &m_DomainCBuffer));
	NAME_D3D_RESOURCE(m_DomainCBuffer, "Tessellated plane domain constant buffer");

	Desc.ByteWidth = sizeof(GeometryCBuffer);

	HFALSE_IF_FAILED(Graphics::GetSingletonPtr()->GetDevice()->CreateBuffer(&Desc, nullptr, &m_GeometryCBuffer));
	NAME_D3D_RESOURCE(m_GeometryCBuffer, "Tessellated plane geometry constant buffer");

	Desc.ByteWidth = sizeof(PlaneInfoCBuffer);

	HFALSE_IF_FAILED(Graphics::GetSingletonPtr()->GetDevice()->CreateBuffer(&Desc, nullptr, &m_PlaneInfoCBuffer));
	NAME_D3D_RESOURCE(m_PlaneInfoCBuffer, "Tessellated plane info constant buffer");
	
	return true;
}

void TessellatedPlane::UpdateBuffers()
{	
	HRESULT hResult;
	D3D11_MAPPED_SUBRESOURCE MappedResource;
	CameraCBuffer* CameraCBufferPtr;
	DomainCBuffer* DomainCBufferPtr;
	PlaneInfoCBuffer* PlaneInfoCBufferPtr;
	ID3D11DeviceContext* DeviceContext = Graphics::GetSingletonPtr()->GetDeviceContext();
	
	std::shared_ptr<Camera> pCamera = Application::GetSingletonPtr()->GetActiveCamera();
	DirectX::XMFLOAT3 CameraPos = pCamera->GetPosition();
	DirectX::XMMATRIX View, Proj;
	pCamera->GetViewMatrix(View);
	Graphics::GetSingletonPtr()->GetProjectionMatrix(Proj);
	
	ASSERT_NOT_FAILED(DeviceContext->Map(m_HullCBuffer.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource));
	CameraCBufferPtr = (CameraCBuffer*)MappedResource.pData;
	CameraCBufferPtr->CameraPos = CameraPos;
	CameraCBufferPtr->TessellationScale = m_TessellationScale;
	DeviceContext->Unmap(m_HullCBuffer.Get(), 0u);

	ASSERT_NOT_FAILED(DeviceContext->Map(m_DomainCBuffer.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource));
	DomainCBufferPtr = (DomainCBuffer*)MappedResource.pData;
	DomainCBufferPtr->ViewProj = DirectX::XMMatrixTranspose(View * Proj);
	DeviceContext->Unmap(m_DomainCBuffer.Get(), 0u);

	GeometryCBuffer CullingBufferData = {};
	PrepCullingBuffer(CullingBufferData);
	ASSERT_NOT_FAILED(DeviceContext->Map(m_GeometryCBuffer.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource));
	memcpy(MappedResource.pData, &CullingBufferData, sizeof(GeometryCBuffer));
	DeviceContext->Unmap(m_GeometryCBuffer.Get(), 0u);

	ASSERT_NOT_FAILED(DeviceContext->Map(m_PlaneInfoCBuffer.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource));
	PlaneInfoCBufferPtr = (PlaneInfoCBuffer*)MappedResource.pData;
	PlaneInfoCBufferPtr->PlaneDimension = (float)m_ChunkDimension * GetScale().x;
	PlaneInfoCBufferPtr->HeightDisplacement = m_HeightDisplacement;
	PlaneInfoCBufferPtr->bVisualiseChunks = m_bVisualiseChunks;
	PlaneInfoCBufferPtr->Padding = 0.f;
	DeviceContext->Unmap(m_PlaneInfoCBuffer.Get(), 0u);
}

void TessellatedPlane::GenerateChunkTransforms(std::vector<DirectX::XMMATRIX>& ChunkTransforms)
{
	float ChunkSize = GetScale().x;
	float ChunkHalf = ChunkSize / 2.f;
	float HalfCount = (float)m_ChunkDimension / 2.f;
	int chunkID = 0;

	for (UINT z = 0; z < m_ChunkDimension; z++)
	{
		float WorldZ = ((int)z - HalfCount) * ChunkSize + ChunkSize * 0.5f;
		for (UINT x = 0; x < m_ChunkDimension; x++)
		{
			float WorldX = ((int)x - HalfCount) * ChunkSize + ChunkSize * 0.5f;
			
			DirectX::XMMATRIX ChunkTransform = DirectX::XMMatrixIdentity();
			ChunkTransform *= DirectX::XMMatrixScaling(ChunkSize, ChunkSize, ChunkSize);
			ChunkTransform *= DirectX::XMMatrixTranslation(WorldX, 0.f, WorldZ);
			ChunkTransforms[chunkID++] = DirectX::XMMatrixTranspose(ChunkTransform);
		}
	}
}

void TessellatedPlane::PrepCullingBuffer(GeometryCBuffer& CullingBufferData, bool bNormalise)
{
	CullingBufferData.FrustumCameraViewProj = DirectX::XMMatrixTranspose(Application::GetSingletonPtr()->GetMainCamera()->GetViewProjMatrix()); // Row-major access

	// Each row of the matrix
	DirectX::XMVECTOR row0 = CullingBufferData.FrustumCameraViewProj.r[0];
	DirectX::XMVECTOR row1 = CullingBufferData.FrustumCameraViewProj.r[1];
	DirectX::XMVECTOR row2 = CullingBufferData.FrustumCameraViewProj.r[2];
	DirectX::XMVECTOR row3 = CullingBufferData.FrustumCameraViewProj.r[3];

	DirectX::XMVECTOR FrustumPlanes[6];
	FrustumPlanes[0] = DirectX::XMVectorAdd(row3, row0); // Left
	FrustumPlanes[1] = DirectX::XMVectorAdd(row3, row0); // Right
	FrustumPlanes[2] = DirectX::XMVectorAdd(row3, row1); // Bottom
	FrustumPlanes[3] = DirectX::XMVectorAdd(row3, row1); // Top
	FrustumPlanes[4] = DirectX::XMVectorAdd(row3, row2); // Near
	FrustumPlanes[5] = DirectX::XMVectorAdd(row3, row2); // Far

	if (bNormalise)
	{
		for (int i = 0; i < 6; ++i)
		{
			FrustumPlanes[i] = DirectX::XMPlaneNormalize(FrustumPlanes[i]);
		}
	}

	for (int i = 0; i < 6; ++i)
	{
		DirectX::XMStoreFloat4(&CullingBufferData.FrustumPlanes[i], FrustumPlanes[i]);
	}
}
