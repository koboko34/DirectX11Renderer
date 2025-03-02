#include "Mesh.h"

#include "Model.h"

Mesh::Mesh(Model* pOwner) : m_pOwner(pOwner)
{
}

void Mesh::ProcessMesh(aiMesh* SceneMesh)
{
	// this is duplicating vertices, come back to this and optimise at some point
	
	m_VerticesOffset = (UINT)m_pOwner->GetVertices().size();
	m_IndicesOffset = (UINT)m_pOwner->GetIndices().size();
	m_MaterialIndex = SceneMesh->mMaterialIndex;

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

		m_pOwner->GetVertices().push_back(v);
	}
	m_VertexCount = (UINT)m_pOwner->GetVertices().size() - m_VerticesOffset;

	for (size_t i = 0; i < SceneMesh->mNumFaces; i++)
	{
		aiFace Face = SceneMesh->mFaces[i];
		for (size_t j = 0; j < Face.mNumIndices; j++)
		{
			m_pOwner->GetIndices().push_back(Face.mIndices[j] + m_VerticesOffset);
		}
	}
	m_IndexCount = (UINT)m_pOwner->GetIndices().size() - m_IndicesOffset;
}