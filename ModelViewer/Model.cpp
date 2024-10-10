#include "Model.h"

#include <fstream>

#include "MyMacros.h"

Model::Model()
{
	m_VertexBuffer = 0;
	m_IndexBuffer = 0;
	m_VertexCount = 0;
	m_IndexCount = 0;
}

Model::Model(const Model& Other)
{
}

Model::~Model()
{
}

bool Model::Initialise(ID3D11Device* Device, ID3D11DeviceContext* DeviceContext, char* ModelFilename)
{
	bool Result;

	FALSE_IF_FAILED(LoadModel(ModelFilename));
	FALSE_IF_FAILED(InitialiseBuffers(Device));
	
	return true;
}

void Model::Shutdown()
{
	ShutdownBuffers();
	ReleaseModel();
}

void Model::Render(ID3D11DeviceContext* DeviceContext)
{
	RenderBuffers(DeviceContext);
}

bool Model::InitialiseBuffers(ID3D11Device* Device)
{
	HRESULT hResult;
	D3D11_BUFFER_DESC vbDesc = {};
	D3D11_BUFFER_DESC ibDesc = {};
	D3D11_SUBRESOURCE_DATA VertexData = {};
	D3D11_SUBRESOURCE_DATA IndexData = {};

	vbDesc.Usage = D3D11_USAGE_IMMUTABLE;
	vbDesc.ByteWidth = sizeof(Vertex) * m_VertexCount;
	vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	VertexData.pSysMem = m_Vertices.data();
	
	HFALSE_IF_FAILED(Device->CreateBuffer(&vbDesc, &VertexData, &m_VertexBuffer));

	ibDesc.Usage = D3D11_USAGE_IMMUTABLE;
	ibDesc.ByteWidth = sizeof(unsigned int) * m_IndexCount;
	ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

	IndexData.pSysMem = m_Indices.data();

	HFALSE_IF_FAILED(Device->CreateBuffer(&ibDesc, &IndexData, &m_IndexBuffer));
	
	return true;
}

void Model::ShutdownBuffers()
{
	if (m_IndexBuffer)
	{
		m_IndexBuffer->Release();
		m_IndexBuffer = 0;
	}

	if (m_VertexBuffer)
	{
		m_VertexBuffer->Release();
		m_VertexBuffer = 0;
	}
}

void Model::RenderBuffers(ID3D11DeviceContext* DeviceContext)
{
	unsigned int Stride, Offset;
	Stride = sizeof(Vertex);
	Offset = 0u;

	DeviceContext->IASetVertexBuffers(0u, 1u, &m_VertexBuffer, &Stride, &Offset);
	DeviceContext->IASetIndexBuffer(m_IndexBuffer, DXGI_FORMAT_R32_UINT, 0u);
	DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

bool Model::LoadModel(char* ModelFilename)
{
	Assimp::Importer Importer;
	auto Model = Importer.ReadFile("Models/suzanne.obj",
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices |
		aiProcess_ConvertToLeftHanded
	);

	if (!Model)
	{
		return false;
	}

	const auto Mesh = Model->mMeshes[0];
	
	m_VertexCount = Mesh->mNumVertices;
	m_Vertices.resize(m_VertexCount);
	for (unsigned int i = 0; i < m_VertexCount; i++)
	{
		Vertex v;
		v.Pos.x = Mesh->mVertices[i].x;
		v.Pos.y = Mesh->mVertices[i].y;
		v.Pos.z = Mesh->mVertices[i].z;
		v.Pos.w = 1.f;

		if (Mesh->HasNormals())
		{
			v.Normal.x = Mesh->mNormals[i].x;
			v.Normal.y = Mesh->mNormals[i].y;
			v.Normal.z = Mesh->mNormals[i].z;
			v.Normal.w = 1.f;
		}

		m_Vertices[i] = v;
	}

	m_IndexCount = Mesh->mNumFaces * 3;
	m_Indices.resize(m_IndexCount);
	for (unsigned int i = 0; i < Mesh->mNumFaces; i++)
	{
		const auto& Face = Mesh->mFaces[i];
		assert(Face.mNumIndices == 3);
		m_Indices[i * 3]	 = Face.mIndices[0];
		m_Indices[i * 3 + 1] = Face.mIndices[1];
		m_Indices[i * 3 + 2] = Face.mIndices[2];
	}

	return true;
}

void Model::ReleaseModel()
{
	m_Vertices.resize(0);
	m_Indices.resize(0);
	m_VertexCount = 0;
	m_IndexCount = 0;
}
