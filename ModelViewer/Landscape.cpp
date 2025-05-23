#include <random>

#include "ImGui/imgui.h"

#include "Landscape.h"
#include "TessellatedPlane.h"
#include "MyMacros.h"
#include "Graphics.h"
#include "Application.h"
#include "Camera.h"
#include "ResourceManager.h"
#include "Common.h"
#include "Grass.h"
#include "FrustumCuller.h"

Landscape::Landscape(UINT ChunkDimension, float ChunkSize, float HeightDisplacement)
	: m_ChunkDimension(ChunkDimension), m_ChunkSize(ChunkSize), m_NumChunks(ChunkDimension * ChunkDimension), m_HeightDisplacement(HeightDisplacement)
{
	SetName("Landscape");
	SetScale(ChunkSize);
	m_ChunkScaleMatrix = DirectX::XMMatrixScaling(ChunkSize, 1.f, ChunkSize);
	m_bShouldRender = true;
	m_bShouldRenderBBoxes = true;
	m_bVisualiseChunks = false;
	m_HeightmapSRV = nullptr;
	m_ChunkInstanceCount = 0u;
	assert(m_NumChunks >= 0 && m_NumChunks <= MAX_PLANE_CHUNKS);
}

Landscape::~Landscape()
{
	Shutdown();
}

bool Landscape::Init(const std::string& HeightMapFilepath, float TessellationScale, UINT GrassDimensionPerChunk)
{
	bool Result;
	FALSE_IF_FAILED(CreateBuffers());

	SetupAABB();
	GenerateChunkOffsets();
	GenerateGrassOffsets(GrassDimensionPerChunk);

	m_Plane = std::make_shared<TessellatedPlane>();
	FALSE_IF_FAILED(m_Plane->Init(TessellationScale, this));

	m_Grass = std::make_shared<Grass>();
	FALSE_IF_FAILED(m_Grass->Init(this, GrassDimensionPerChunk));

	m_HeightmapSRV = ResourceManager::GetSingletonPtr()->LoadTexture(HeightMapFilepath);
	assert(m_HeightmapSRV);

	m_HeightMapFilepath = HeightMapFilepath;

	return true;
}

void Landscape::SetupAABB()
{
	m_BoundingBox.Min = { -0.5f, 0.f, -0.5 };
	m_BoundingBox.Max = { 0.5f, m_HeightDisplacement, 0.5f };
	m_BoundingBox.CalcCorners();
}

void Landscape::Render()
{	
	Application* pApp = Application::GetSingletonPtr();
	pApp->GetFrustumCuller()->DispatchShader(GetChunkOffsets(), m_BoundingBox.Corners, m_ChunkScaleMatrix);
	m_ChunkInstanceCount = pApp->GetFrustumCuller()->GetInstanceCount();
	
	if (m_ChunkInstanceCount == 0u)
		return;

	UpdateBuffers();

	if (m_Plane->ShouldRender())
	{
		m_Plane->Render();
	}

	if (m_Grass->ShouldRender())
	{
		m_Grass->Render();
	}
}

void Landscape::Shutdown()
{
	m_LandscapeInfoCBuffer.Reset();
	m_CameraCBuffer.Reset();
	m_Plane.reset();
	m_Grass.reset();

	ResourceManager::GetSingletonPtr()->UnloadTexture(m_HeightMapFilepath);
	m_HeightmapSRV = nullptr;
}

void Landscape::RenderControls()
{
	ImGui::Text("Landscape");
	
	float HeightDisplacement = m_HeightDisplacement;
	if (ImGui::DragFloat("Height Displacement", &HeightDisplacement, 0.1f, 0.f, 256.f, "%.f", ImGuiSliderFlags_AlwaysClamp))
	{
		SetHeightDisplacement(HeightDisplacement);
	}

	ImGui::Checkbox("Should Render?", &m_bShouldRender);
	ImGui::Checkbox("Should Render Bounding Boxes?", &m_bShouldRenderBBoxes);
	ImGui::Checkbox("Visualise Chunks?", &m_bVisualiseChunks);

	ImGui::Dummy(ImVec2(0.f, 10.f));

	m_Plane->RenderControls();

	ImGui::Dummy(ImVec2(0.f, 10.f));

	m_Grass->RenderControls();
}

void Landscape::SetHeightDisplacement(float NewHeight)
{
	m_HeightDisplacement = NewHeight;
	SetupAABB();
}

bool Landscape::CreateBuffers()
{
	HRESULT hResult;
	D3D11_BUFFER_DESC Desc = {};

	Desc.Usage = D3D11_USAGE_DYNAMIC;
	Desc.ByteWidth = sizeof(CullingCBuffer);
	Desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	Desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

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
	LandscapeInfoCBufferPtr->ChunkInstanceCount = m_ChunkInstanceCount;
	LandscapeInfoCBufferPtr->GrassPerChunk = m_Grass->GetGrassPerChunk();
	LandscapeInfoCBufferPtr->Time = (float)Application::GetSingletonPtr()->GetAppTime();
	LandscapeInfoCBufferPtr->Padding = {};
	LandscapeInfoCBufferPtr->ChunkScaleMatrix = DirectX::XMMatrixTranspose(DirectX::XMMatrixScaling(m_ChunkSize, m_ChunkSize, m_ChunkSize));

	DeviceContext->Unmap(m_LandscapeInfoCBuffer.Get(), 0u);
}

void Landscape::GenerateChunkOffsets()
{
	float ChunkHalf = m_ChunkSize / 2.f;
	float HalfCount = (float)m_ChunkDimension / 2.f;
	int chunkID = 0;

	m_ChunkOffsets.resize(m_NumChunks, { 0.f, 0.f });
	for (UINT z = 0; z < m_ChunkDimension; z++)
	{
		float WorldZ = ((int)z - HalfCount) * m_ChunkSize + m_ChunkSize * 0.5f;
		for (UINT x = 0; x < m_ChunkDimension; x++)
		{
			float WorldX = ((int)x - HalfCount) * m_ChunkSize + m_ChunkSize * 0.5f;

			m_ChunkOffsets[chunkID++] = { WorldX, WorldZ };
		}
	}
}

void Landscape::GenerateGrassOffsets(UINT GrassCount)
{
	if (GrassCount <= 1)
	{
		m_GrassOffsets.push_back({ 0.f, 0.f });
		return;
	}

	float SpacingX = m_ChunkSize / (GrassCount);
	float SpacingZ = m_ChunkSize / (GrassCount);

	float HalfWidth = m_ChunkSize * 0.5f;
	float HalfDepth = m_ChunkSize * 0.5f;

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<float> TranslationDist(0.f, SpacingX);
	std::uniform_real_distribution<float> RotationDist(0.f, 360.f);

	m_GrassOffsets.resize(MAX_GRASS_PER_CHUNK, { 0.f, 0.f });
	int i = 0;
	for (UINT x = 0; x < GrassCount; ++x)
	{
		for (UINT z = 0; z < GrassCount; ++z)
		{
			float RandOffsetX = TranslationDist(gen);
			float RandOffsetZ = TranslationDist(gen);
			
			float WorldX = -HalfWidth + x * SpacingX + RandOffsetX;
			float WorldZ = -HalfDepth + z * SpacingZ + RandOffsetZ;

			assert(i < MAX_GRASS_PER_CHUNK);

			m_GrassOffsets[i] = { WorldX, WorldZ };
			i++;
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
