#include "Model.h"

#include <fstream>

#include "MyMacros.h"
#include "Application.h"
#include "ResourceManager.h"
#include "InstancedShader.h"

Model::Model()
{
	m_ModelPath = "";
	m_TexturesPath = "";
}

Model::Model(const Model& Other)
{
}

Model::~Model()
{
	Shutdown();
}

bool Model::Initialise(ID3D11Device* Device, ID3D11DeviceContext* DeviceContext, std::string ModelFilename, std::string TexturesPath)
{
	bool Result;
	m_ModelPath = ModelFilename;
	m_TexturesPath = TexturesPath;

	FALSE_IF_FAILED(LoadModel());
	
	return true;
}

void Model::Shutdown()
{
	Reset();
}

void Model::Render(ID3D11Buffer* InstanceBuffer)
{
	ID3D11DeviceContext* DeviceContext = Application::GetSingletonPtr()->GetGraphics()->GetDeviceContext();
	UINT Strides[2] = { sizeof(Vertex), sizeof(InstanceData) };
	UINT Offsets[2] = { 0u, 0u };
	ID3D11Buffer* Buffers[2] = { m_VertexBuffer.Get(), InstanceBuffer };

	DeviceContext->IASetVertexBuffers(0u, 2u, Buffers, Strides, Offsets);
	DeviceContext->IASetIndexBuffer(m_IndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0u);
	DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	Application::GetSingletonPtr()->GetGraphics()->EnableDepthWrite();
	Application::GetSingletonPtr()->GetGraphics()->DisableBlending();
	RenderMeshesInstanced(m_OpaqueMeshes);
	Application::GetSingletonPtr()->GetGraphics()->DisableDepthWrite();
	Application::GetSingletonPtr()->GetGraphics()->EnableBlending();
	RenderMeshesInstanced(m_TransparentMeshes);
}

void Model::ShutdownBuffers()
{
	m_VertexBuffer.Reset();
	m_IndexBuffer.Reset();
}

bool Model::LoadModel()
{
	Reset();
	
	Assimp::Importer Importer;
	const aiScene* Scene = Importer.ReadFile(m_ModelPath,
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices |
		aiProcess_GenSmoothNormals |
		//aiProcess_FlipWindingOrder
		//aiProcess_MakeLeftHanded
		//aiProcess_FlipUVs
		aiProcess_ConvertToLeftHanded
	);

	assert(Scene);

	LoadMaterials(Scene);
	m_RootNode = std::make_unique<Node>(this, nullptr);
	m_RootNode->ProcessNode(Scene->mRootNode, Scene, DirectX::XMMatrixIdentity());
	CreateBuffers();

	return true;
}

void Model::ReleaseModel()
{
	m_RootNode.reset();

	m_Transforms.clear();
	m_Textures.clear();
	m_Materials.clear();
	m_OpaqueMeshes.clear();
	m_TransparentMeshes.clear();
	m_Vertices.clear();
	m_Indices.clear();

	for (const std::string& Path : m_TexturePathsSet)
	{
		ResourceManager::GetSingletonPtr()->UnloadResource(Path);
	}
	m_TexturePathsSet.clear();

	m_Textures.shrink_to_fit();
	m_Materials.shrink_to_fit();
	m_OpaqueMeshes.shrink_to_fit();
	m_TransparentMeshes.shrink_to_fit();
	m_Vertices.shrink_to_fit();
	m_Indices.shrink_to_fit();
}

void Model::Reset()
{
	ShutdownBuffers();
	m_TextureIndexMap.clear();
	ReleaseModel();
}

bool Model::CreateBuffers()
{
	HRESULT hResult;
	D3D11_BUFFER_DESC vbDesc = {};
	D3D11_SUBRESOURCE_DATA VertexData = {};

	vbDesc.Usage = D3D11_USAGE_IMMUTABLE;
	vbDesc.ByteWidth = (UINT)(sizeof(Vertex) * m_Vertices.size());
	vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	VertexData.pSysMem = m_Vertices.data();

	HFALSE_IF_FAILED(Application::GetSingletonPtr()->GetGraphics()->GetDevice()->CreateBuffer(&vbDesc, &VertexData, &m_VertexBuffer));

	D3D11_BUFFER_DESC ibDesc = {};
	D3D11_SUBRESOURCE_DATA IndexData = {};

	ibDesc.Usage = D3D11_USAGE_IMMUTABLE;
	ibDesc.ByteWidth = (UINT)(sizeof(unsigned int) * m_Indices.size());
	ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

	IndexData.pSysMem = m_Indices.data();

	HFALSE_IF_FAILED(Application::GetSingletonPtr()->GetGraphics()->GetDevice()->CreateBuffer(&ibDesc, &IndexData, &m_IndexBuffer));

	return true;
}

void Model::LoadMaterials(const aiScene* Scene)
{
	for (size_t i = 0; i < Scene->mNumMaterials; i++)
	{
		m_Materials.emplace_back(std::make_shared<Material>((UINT)i, this));
		m_Materials.back()->LoadTextures(Scene->mMaterials[i]);
		m_Materials.back()->CreateConstantBuffer();
	}
}

void Model::RenderMeshes(const std::vector<std::unique_ptr<Mesh>>& Meshes)
{
	ID3D11DeviceContext* DeviceContext = Application::GetSingletonPtr()->GetGraphics()->GetDeviceContext();

	for (const std::unique_ptr<Mesh>& m : Meshes)
	{
		std::shared_ptr<Material> Mat = m.get()->m_Material;

		DeviceContext->VSSetConstantBuffers(1u, 1u, m->m_pNode->m_ConstantBuffer.GetAddressOf());
		DeviceContext->PSSetConstantBuffers(1u, 1u, Mat->m_ConstantBuffer.GetAddressOf());

		if (Mat->m_DiffuseSRV >= 0)
		{
			DeviceContext->PSSetShaderResources(0u, 1u, m_Textures[Mat->m_DiffuseSRV].GetAddressOf());
		}

		if (Mat->m_SpecularSRV >= 0)
		{
			DeviceContext->PSSetShaderResources(1u, 1u, m_Textures[Mat->m_SpecularSRV].GetAddressOf());
		}

		//Application::GetSingletonPtr()->GetGraphics()->SetRasterStateBackFaceCull(!Mat->m_bTwoSided); // this doesn't actually work in some cases, investigate
		Application::GetSingletonPtr()->GetGraphics()->SetRasterStateBackFaceCull(true);

		DeviceContext->DrawIndexed(m->m_IndexCount, m->m_IndicesOffset, 0);
	}
}

void Model::RenderMeshesInstanced(const std::vector<std::unique_ptr<Mesh>>& Meshes)
{
	ID3D11DeviceContext* DeviceContext = Application::GetSingletonPtr()->GetGraphics()->GetDeviceContext();

	for (const std::unique_ptr<Mesh>& m : Meshes)
	{
		std::shared_ptr<Material> Mat = m.get()->m_Material;

		DeviceContext->VSSetConstantBuffers(1u, 1u, m->m_pNode->m_ConstantBuffer.GetAddressOf());
		DeviceContext->PSSetConstantBuffers(1u, 1u, Mat->m_ConstantBuffer.GetAddressOf());

		if (Mat->m_DiffuseSRV >= 0)
		{
			DeviceContext->PSSetShaderResources(0u, 1u, m_Textures[Mat->m_DiffuseSRV].GetAddressOf());
		}

		if (Mat->m_SpecularSRV >= 0)
		{
			DeviceContext->PSSetShaderResources(1u, 1u, m_Textures[Mat->m_SpecularSRV].GetAddressOf());
		}

		//Application::GetSingletonPtr()->GetGraphics()->SetRasterStateBackFaceCull(!Mat->m_bTwoSided); // this doesn't actually work in some cases, investigate
		Application::GetSingletonPtr()->GetGraphics()->SetRasterStateBackFaceCull(true);

		DeviceContext->DrawIndexedInstanced(m->m_IndexCount, (UINT)GetTransforms().size(), m->m_IndicesOffset, 0, 0);
	}
}

