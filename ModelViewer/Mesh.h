#pragma once

#ifndef MESH_H
#define MESH_H

#include "DirectXMath.h"

#include <memory>
#include <string>

struct aiMesh;
class Material;

class Mesh
{
	friend class Model;
	friend class Node;

public:
	Mesh(Model* pModel, Node* pNode);

	void ProcessMesh(aiMesh* SceneMesh);

private:
	unsigned int m_VerticesOffset;
	unsigned int m_IndicesOffset;
	unsigned int m_VertexCount;
	unsigned int m_IndexCount;
	std::shared_ptr<Material> m_Material;
	std::string m_Name;

	Model* m_pModel;
	Node* m_pNode;

};

#endif
