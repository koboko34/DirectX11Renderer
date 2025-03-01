#pragma once

#ifndef MESH_H
#define MESH_H

#include "DirectXMath.h"

struct aiMesh;

class Mesh
{
	friend class Model;

public:
	Mesh(Model* pOwner);

	void ProcessMesh(aiMesh* SceneMesh);

private:
	unsigned int m_VerticesOffset;
	unsigned int m_IndicesOffset;
	unsigned int m_VertexCount;
	unsigned int m_IndexCount;
	unsigned int m_MaterialIndex;

	Model* m_pOwner;

};

#endif
