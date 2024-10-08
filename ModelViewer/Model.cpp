#include "Model.h"

#include <fstream>

#include "MyMacros.h"

Model::Model()
{
	m_VertexBuffer = 0;
	m_IndexBuffer = 0;
	m_Vertices = 0;
	m_Indices = 0;
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

	VertexData.pSysMem = m_Vertices;
	
	HFALSE_IF_FAILED(Device->CreateBuffer(&vbDesc, &VertexData, &m_VertexBuffer));

	ibDesc.Usage = D3D11_USAGE_IMMUTABLE;
	ibDesc.ByteWidth = sizeof(unsigned int) * m_IndexCount;
	ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

	IndexData.pSysMem = m_Indices;

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
	// temporary, convert to loading with assimp later
	
	m_VertexCount = 3;
	m_IndexCount = 3;

	m_Vertices = new Vertex[m_VertexCount];
	m_Indices = new unsigned int[m_IndexCount];

	m_Vertices[0].Pos = DirectX::XMFLOAT3(-1.f, -1.f, 0.f);
	m_Vertices[0].Padding = 0.f;
	m_Vertices[1].Pos = DirectX::XMFLOAT3( 0.f,  1.f, 0.f);
	m_Vertices[1].Padding = 0.f;
	m_Vertices[2].Pos = DirectX::XMFLOAT3( 1.f, -1.f, 0.f);
	m_Vertices[2].Padding = 0.f;

	m_Indices[0] = 0;
	m_Indices[1] = 1;
	m_Indices[2] = 2;

	return true;

	// ==============================================
	/*
	std::ifstream fin;
	char Input;
	int i;

	fin.open(ModelFilename);
	if (fin.fail())
	{
		return false;
	}

	fin.get(Input);
	while (Input != ':')
	{
		fin.get(Input);
	}

	fin >> m_VertexCount;

	m_IndexCount = m_VertexCount;

	// m_Vertices = new Vertex[m_VertexCount];
	// m_Indices = new unsigned int[m_IndexCount];

	fin.get(Input);
	while (Input != ':')
	{
		fin.get(Input);
	}
	fin.get(Input);
	fin.get(Input);

	for (i = 0; i < m_VertexCount; i++)
	{
		fin >> m_Vertices[i].Pos.x >> m_Vertices[i].Pos.y >> m_Vertices[i].Pos.z;
		m_Indices[i] = i;
	}

	fin.close();

	return true;
	*/
}

void Model::ReleaseModel()
{
	if (m_Vertices)
	{
		delete[] m_Vertices;
		m_Vertices = 0;
	}

	if (m_Indices)
	{
		delete[] m_Indices;
		m_Indices = 0;
	}
}
