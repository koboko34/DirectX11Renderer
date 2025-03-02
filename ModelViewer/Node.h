#pragma once

#ifndef NODE_H
#define NODE_H

#include <vector>
#include <memory>

#include "Mesh.h"

struct aiNode;
struct aiScene;

class Node
{
	friend class Model;

public:
	Node(Model* pModel, Node* pOwner);

	void ProcessNode(aiNode* ModelNode, const aiScene* Scene);

	std::vector<std::unique_ptr<Mesh>>& GetMeshes() { return m_Meshes; }

private:
	void RenderMeshes();

private:
	std::vector<std::unique_ptr<Mesh>> m_Meshes;
	std::vector<std::unique_ptr<Node>> m_Children;

	Model* m_pModel;
	Node* m_pOwner;
};

#endif
