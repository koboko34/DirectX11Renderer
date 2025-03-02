#include "Mesh.h"

#include "Model.h"

Mesh::Mesh(Model* pModel, Node* pNode) : m_pModel(pModel), m_pNode(pNode)
{
}

void Mesh::ProcessMesh(aiMesh* SceneMesh)
{
	// this is duplicating vertices, come back to this and optimise at some point
	
	m_VerticesOffset = (UINT)m_pModel->GetVertices().size();
	m_IndicesOffset = (UINT)m_pModel->GetIndices().size();
	m_Material = m_pModel->GetMaterials()[SceneMesh->mMaterialIndex];

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

		m_pModel->GetVertices().push_back(v);
	}
	m_VertexCount = (UINT)m_pModel->GetVertices().size() - m_VerticesOffset;

	for (size_t i = 0; i < SceneMesh->mNumFaces; i++)
	{
		aiFace Face = SceneMesh->mFaces[i];
		for (size_t j = 0; j < Face.mNumIndices; j++)
		{
			m_pModel->GetIndices().push_back(Face.mIndices[j] + m_VerticesOffset);
		}
	}
	m_IndexCount = (UINT)m_pModel->GetIndices().size() - m_IndicesOffset;
}