#include "ImGui/imgui.h"

#include "Landscape.h"
#include "TessellatedPlane.h"
#include "MyMacros.h"
#include "Graphics.h"
#include "Application.h"
#include "Camera.h"
#include "ResourceManager.h"
#include "Common.h"

Landscape::Landscape(UINT ChunkDimension, float ChunkSize, float HeightDisplacement)
	: m_ChunkDimension(ChunkDimension), m_ChunkSize(ChunkSize), m_NumChunks(ChunkDimension* ChunkDimension), m_HeightDisplacement(HeightDisplacement)
{
	SetName("Landscape");
	SetScale(ChunkSize);
	m_bShouldRender = true;
	m_bVisualiseChunks = false;
	assert(m_NumChunks >= 0 && m_NumChunks <= MAX_PLANE_CHUNKS);
}

Landscape::~Landscape()
{
	Shutdown();
}

bool Landscape::Init(const std::string& HeightMapFilepath, float TessellationScale)
{
	bool Result;
	FALSE_IF_FAILED(CreateBuffers());

	m_Plane = std::make_unique<TessellatedPlane>();
	FALSE_IF_FAILED(m_Plane->Init(HeightMapFilepath, TessellationScale, this));

	return true;
}

void Landscape::Render()
{
	Graphics::GetSingletonPtr()->EnableDepthWrite();
	Graphics::GetSingletonPtr()->DisableBlending();

	UpdateBuffers();

	if (m_Plane->ShouldRender())
	{
		m_Plane->Render();
	}
}

void Landscape::Shutdown()
{
	m_ChunkTransformsCBuffer.Reset();
	m_LandscapeInfoCBuffer.Reset();
	m_CameraCBuffer.Reset();
	m_Plane.reset();
}

void Landscape::RenderControls()
{
	ImGui::Text("Landscape");
	
	ImGui::DragFloat("Height Displacement", &m_HeightDisplacement, 0.1f, 0.f, 256.f, "%.f", ImGuiSliderFlags_AlwaysClamp);

	ImGui::Checkbox("Should Render?", &m_bShouldRender);
	ImGui::Checkbox("Visualise Chunks?", &m_bVisualiseChunks);

	ImGui::Dummy(ImVec2(0.f, 10.f));

	m_Plane->RenderControls();
}

bool Landscape::CreateBuffers()
{
	HRESULT hResult;
	D3D11_BUFFER_DESC Desc = {};

	Desc.Usage = D3D11_USAGE_IMMUTABLE;
	Desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	Desc.ByteWidth = sizeof(TransformCBuffer) * MAX_PLANE_CHUNKS;

	std::vector<DirectX::XMMATRIX> ChunkTransforms(MAX_PLANE_CHUNKS, DirectX::XMMatrixIdentity());
	GenerateChunkTransforms(ChunkTransforms);
	D3D11_SUBRESOURCE_DATA Data = {};
	Data.pSysMem = ChunkTransforms.data();

	HFALSE_IF_FAILED(Graphics::GetSingletonPtr()->GetDevice()->CreateBuffer(&Desc, &Data, &m_ChunkTransformsCBuffer));
	NAME_D3D_RESOURCE(m_ChunkTransformsCBuffer, "Landscape chunk transforms constant buffer");

	Desc.Usage = D3D11_USAGE_DYNAMIC;
	Desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	Desc.ByteWidth = sizeof(CullingCBuffer);

	HFALSE_IF_FAILED(Graphics::GetSingletonPtr()->GetDevice()->CreateBuffer(&Desc, nullptr, &m_CullingCBuffer));
	NAME_D3D_RESOURCE(m_CullingCBuffer, "Landscape culling constant buffer");

	Desc.ByteWidth = sizeof(LandscapeInfoCBuffer);

	HFALSE_IF_FAILED(Graphics::GetSingletonPtr()->GetDevice()->CreateBuffer(&Desc, nullptr, &m_LandscapeInfoCBuffer));
	NAME_D3D_RESOURCE(m_LandscapeInfoCBuffer, "Landscape info constant buffer");

	Desc.ByteWidth = sizeof(CameraCBuffer);

	HFALSE_IF_FAILED(Graphics::GetSingletonPtr()->GetDevice()->CreateBuffer(&Desc, nullptr, &m_CameraCBuffer));
	NAME_D3D_RESOURCE(m_CameraCBuffer, "Landscape camera constant buffer");

	return true;
}

void Landscape::UpdateBuffers()
{
	HRESULT hResult;
	D3D11_MAPPED_SUBRESOURCE MappedResource;
	CameraCBuffer* CameraCBufferPtr;
	LandscapeInfoCBuffer* LandscapeInfoCBufferPtr;
	ID3D11DeviceContext* DeviceContext = Graphics::GetSingletonPtr()->GetDeviceContext();

	std::shared_ptr<Camera> pCamera = Application::GetSingletonPtr()->GetActiveCamera();
	DirectX::XMFLOAT3 CameraPos = pCamera->GetPosition();
	DirectX::XMMATRIX View, Proj;
	pCamera->GetViewMatrix(View);
	Graphics::GetSingletonPtr()->GetProjectionMatrix(Proj);

	ASSERT_NOT_FAILED(DeviceContext->Map(m_CameraCBuffer.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource));
	CameraCBufferPtr = (CameraCBuffer*)MappedResource.pData;
	CameraCBufferPtr->ViewProj = DirectX::XMMatrixTranspose(View * Proj);
	DeviceContext->Unmap(m_CameraCBuffer.Get(), 0u);

	CullingCBuffer CullingBufferData = {};
	PrepCullingBuffer(CullingBufferData);
	ASSERT_NOT_FAILED(DeviceContext->Map(m_CullingCBuffer.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource));
	memcpy(MappedResource.pData, &CullingBufferData, sizeof(CullingCBuffer));
	DeviceContext->Unmap(m_CullingCBuffer.Get(), 0u);

	ASSERT_NOT_FAILED(DeviceContext->Map(m_LandscapeInfoCBuffer.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource));
	LandscapeInfoCBufferPtr = (LandscapeInfoCBuffer*)MappedResource.pData;
	LandscapeInfoCBufferPtr->PlaneDimension = (float)m_ChunkDimension * m_ChunkSize;
	LandscapeInfoCBufferPtr->HeightDisplacement = m_HeightDisplacement;
	LandscapeInfoCBufferPtr->bVisualiseChunks = m_bVisualiseChunks;
	LandscapeInfoCBufferPtr->Padding = 0.f;
	DeviceContext->Unmap(m_LandscapeInfoCBuffer.Get(), 0u);
}

void Landscape::GenerateChunkTransforms(std::vector<DirectX::XMMATRIX>& ChunkTransforms)
{
	float ChunkHalf = m_ChunkSize / 2.f;
	float HalfCount = (float)m_ChunkDimension / 2.f;
	int chunkID = 0;

	for (UINT z = 0; z < m_ChunkDimension; z++)
	{
		float WorldZ = ((int)z - HalfCount) * m_ChunkSize + m_ChunkSize * 0.5f;
		for (UINT x = 0; x < m_ChunkDimension; x++)
		{
			float WorldX = ((int)x - HalfCount) * m_ChunkSize + m_ChunkSize * 0.5f;

			DirectX::XMMATRIX ChunkTransform = DirectX::XMMatrixIdentity();
			ChunkTransform *= DirectX::XMMatrixScaling(m_ChunkSize, m_ChunkSize, m_ChunkSize);
			ChunkTransform *= DirectX::XMMatrixTranslation(WorldX, 0.f, WorldZ);
			ChunkTransforms[chunkID++] = DirectX::XMMatrixTranspose(ChunkTransform);
		}
	}
}

void Landscape::PrepCullingBuffer(CullingCBuffer& CullingBufferData, bool bNormalise)
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
