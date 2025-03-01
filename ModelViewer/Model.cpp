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

	for (const Mesh& m : m_Meshes)
	{
		DeviceContext->PSSetConstantBuffers(0u, 1u, m_Materials[1].ConstantBuffer.GetAddressOf());
		
		if (m_Materials[1].DiffuseSRV >= 0)
		{
			DeviceContext->PSSetShaderResources(0u, 1u, m_TexturesMap[m.MaterialIndex].GetAddressOf());
		}

		DeviceContext->DrawIndexed(m.IndexCount, m.IndicesOffset, 0);
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
		aiProcess_ConvertToLeftHanded
	);

	if (!Scene)
	{
		return false;
	}

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
		aiMesh* Mesh = Scene->mMeshes[Node->mMeshes[i]];
		ProcessMesh(Mesh, Scene);
	}

	for (size_t i = 0; i < Node->mNumChildren; i++)
	{
		ProcessNode(Node->mChildren[i], Scene);
	}
}

void Model::ProcessMesh(aiMesh* SceneMesh, const aiScene* Scene)
{
	Mesh CurrentMesh = {};
	CurrentMesh.VerticesOffset = m_Vertices.size();
	CurrentMesh.IndicesOffset = m_Indices.size();
	CurrentMesh.MaterialIndex = SceneMesh->mMaterialIndex;
	
	for (size_t i = 0; i < SceneMesh->mNumVertices; i++)
	{
		Vertex v;
		v.Pos = DirectX::XMFLOAT3(SceneMesh->mVertices[i].x, SceneMesh->mVertices[i].y, SceneMesh->mVertices[i].z);
		v.Normal = DirectX::XMFLOAT3(SceneMesh->mNormals[i].x, SceneMesh->mNormals[i].y, SceneMesh->mNormals[i].z);

		if (SceneMesh->mTextureCoords[0])
		{
			v.TexCoord = DirectX::XMFLOAT2(SceneMesh->mTextureCoords[0][i].x, SceneMesh->mTextureCoords[0][i].y);
		}
		else
		{
			v.TexCoord = DirectX::XMFLOAT2(0.f, 0.f);
		}

		m_Vertices.push_back(v);
	}
	CurrentMesh.VertexCount = m_Vertices.size() - CurrentMesh.VerticesOffset;

	for (size_t i = 0; i < SceneMesh->mNumFaces; i++)
	{
		aiFace Face = SceneMesh->mFaces[i];
		for (size_t j = 0; j < Face.mNumIndices; j++)
		{
			m_Indices.push_back(Face.mIndices[j]);
		}
	}
	CurrentMesh.IndexCount = m_Indices.size() - CurrentMesh.IndicesOffset;

	m_Meshes.push_back(CurrentMesh);
}

bool Model::CreateBuffers()
{
	HRESULT hResult;
	D3D11_BUFFER_DESC vbDesc = {};
	D3D11_SUBRESOURCE_DATA VertexData = {};

	vbDesc.Usage = D3D11_USAGE_IMMUTABLE;
	vbDesc.ByteWidth = sizeof(Vertex) * m_Vertices.size();
	vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	VertexData.pSysMem = m_Vertices.data();

	HFALSE_IF_FAILED(Application::GetSingletonPtr()->GetGraphics()->GetDevice()->CreateBuffer(&vbDesc, &VertexData, &m_VertexBuffer));

	D3D11_BUFFER_DESC ibDesc = {};
	D3D11_SUBRESOURCE_DATA IndexData = {};

	ibDesc.Usage = D3D11_USAGE_IMMUTABLE;
	ibDesc.ByteWidth = sizeof(unsigned int) * m_Indices.size();
	ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

	IndexData.pSysMem = m_Indices.data();

	HFALSE_IF_FAILED(Application::GetSingletonPtr()->GetGraphics()->GetDevice()->CreateBuffer(&ibDesc, &IndexData, &m_IndexBuffer));

	return true;
}

void Model::LoadMaterials(const aiScene* Scene)
{
	for (size_t i = 0; i < Scene->mNumMaterials; i++)
	{
		Material Mat;
		LoadTexture(Scene->mMaterials[i], i, Mat);
		CreateConstantBuffer(Mat);

		m_Materials.push_back(Mat);
	}
}

void Model::LoadTexture(aiMaterial* ModelMaterial, unsigned int Index, Material& Mat)
{
	if (m_TexturesMap.find(Index) != m_TexturesMap.end())
	{
		return;
	}
	
	aiString Path;
	aiColor3D Color;
	if (ModelMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &Path) == AI_SUCCESS)
	{
		std::string FullPath = m_TexturesPath + std::string(Path.C_Str());

		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> SRV = Application::GetSingletonPtr()->GetGraphics()->LoadTexture(FullPath.c_str());
		m_TexturesMap.insert({ Index, SRV });
		Mat.DiffuseSRV = (int)Index;
	}
	else if (ModelMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, Color) == AI_SUCCESS)
	{
		Mat.DiffuseColor.x = Color.r;
		Mat.DiffuseColor.y = Color.g;
		Mat.DiffuseColor.z = Color.b;
	}
}

void Model::CreateConstantBuffer(Material& Mat)
{
	HRESULT hResult;
	D3D11_BUFFER_DESC BufferDesc = {};
	BufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	BufferDesc.ByteWidth = sizeof(Material::MaterialData);
	BufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	Material::MaterialData Data = {};
	Data.DiffuseColor = Mat.DiffuseColor;
	Data.DiffuseSRV = Mat.DiffuseSRV;

	D3D11_SUBRESOURCE_DATA BufferData = {};
	BufferData.pSysMem = &Data;

	ASSERT_NOT_FAILED(Application::GetSingletonPtr()->GetGraphics()->GetDevice()->CreateBuffer(&BufferDesc, &BufferData, &Mat.ConstantBuffer));
}
