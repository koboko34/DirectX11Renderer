#include "Model.h"

#include <fstream>

#include "MyMacros.h"
#include "Application.h"

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
	ShutdownBuffers();
	ReleaseModel();
}

void Model::Render(ID3D11DeviceContext* DeviceContext)
{
	RenderMeshes(DeviceContext);
}

void Model::ShutdownBuffers()
{
	m_VertexBuffer.Reset();
	m_IndexBuffer.Reset();
}

void Model::RenderMeshes(ID3D11DeviceContext* DeviceContext)
{
	UINT Stride = sizeof(Vertex);
	UINT Offset = 0u;
	
	DeviceContext->IASetVertexBuffers(0u, 1u, m_VertexBuffer.GetAddressOf(), &Stride, &Offset);
	DeviceContext->IASetIndexBuffer(m_IndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0u);
	DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	for (const std::unique_ptr<Mesh>& m : m_Meshes)
	{
		DeviceContext->PSSetConstantBuffers(0u, 1u, m_Materials[m->m_MaterialIndex]->m_ConstantBuffer.GetAddressOf());
		
		if (m_Materials[m->m_MaterialIndex]->m_DiffuseSRV >= 0)
		{
			DeviceContext->PSSetShaderResources(0u, 1u, m_TexturesMap[m->m_MaterialIndex].GetAddressOf());
		}

		Application::GetSingletonPtr()->GetGraphics()->SetRasterStateBackFaceCull(!m_Materials[m->m_MaterialIndex]->m_bTwoSided);

		DeviceContext->DrawIndexed(m->m_IndexCount, m->m_IndicesOffset, 0);
	}
}

bool Model::LoadModel()
{
	Reset();
	
	Assimp::Importer Importer;
	const aiScene* Scene = Importer.ReadFile(m_ModelPath,
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices |
		aiProcess_GenSmoothNormals |
		aiProcess_MakeLeftHanded |
		aiProcess_FlipUVs
		//aiProcess_ConvertToLeftHanded
	);

	assert(Scene);

	ProcessNode(Scene->mRootNode, Scene);
	CreateBuffers();
	LoadMaterials(Scene);

	return true;
}

void Model::ReleaseModel()
{
	m_Vertices.resize(0);
	m_Indices.resize(0);
}

void Model::Reset()
{
	ShutdownBuffers();
	m_TexturesMap.clear();
	m_Materials.clear();
	m_Meshes.clear();
	ReleaseModel();
}

void Model::ProcessNode(aiNode* Node, const aiScene* Scene)
{
	for (size_t i = 0; i < Node->mNumMeshes; i++)
	{
		m_Meshes.emplace_back(std::make_unique<Mesh>(this));
		UINT MeshIndex = (UINT)m_Meshes.size() - 1;
		
		aiMesh* SceneMesh = Scene->mMeshes[Node->mMeshes[i]];		
		m_Meshes[MeshIndex]->ProcessMesh(SceneMesh);
	}

	for (size_t i = 0; i < Node->mNumChildren; i++)
	{
		ProcessNode(Node->mChildren[i], Scene);
	}
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
		m_Materials.emplace_back(std::make_unique<Material>((UINT)i, this));
		UINT MatIndex = (UINT)m_Materials.size() - 1;
		m_Materials[MatIndex]->LoadTextures(Scene->mMaterials[i]);
		m_Materials[MatIndex]->CreateConstantBuffer();
	}
}
