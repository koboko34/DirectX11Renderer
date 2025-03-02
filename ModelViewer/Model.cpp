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
	Reset();
}

void Model::Render()
{
	m_RootNode->RenderMeshes();
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
		aiProcess_MakeLeftHanded |
		aiProcess_FlipUVs
		//aiProcess_ConvertToLeftHanded
	);

	assert(Scene);

	LoadMaterials(Scene);
	m_RootNode = std::make_unique<Node>(this, nullptr);
	m_RootNode->ProcessNode(Scene->mRootNode, Scene);
	CreateBuffers();

	return true;
}

void Model::ReleaseModel()
{
	m_RootNode.reset();

	m_Textures.clear();
	m_Materials.clear();
	m_Meshes.clear();
	m_Vertices.clear();
	m_Indices.clear();

	m_Textures.shrink_to_fit();
	m_Materials.shrink_to_fit();
	m_Meshes.shrink_to_fit();
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
		UINT MatIndex = (UINT)m_Materials.size() - 1;
		m_Materials[MatIndex]->LoadTextures(Scene->mMaterials[i]);
		m_Materials[MatIndex]->CreateConstantBuffer();
	}
}
