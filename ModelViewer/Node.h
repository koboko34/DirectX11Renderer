#pragma once

#ifndef NODE_H
#define NODE_H

#include <vector>
#include <memory>
#include <string>

#include "DirectXMath.h"

#include "d3d11.h"

#include "wrl.h"

#include "assimp/matrix4x4.h"

#include "Mesh.h"

struct aiNode;
struct aiScene;

class Node
{
	friend class ModelData;

public:
	Node(ModelData* pModel, Node* pOwner);

	void ProcessNode(aiNode* ModelNode, const aiScene* Scene, const DirectX::XMMATRIX& AccumulatedTransform);

private:
	void CreateConstantBuffer();

	DirectX::XMMATRIX ConvertToXMMATRIX(const aiMatrix4x4& aiMatrix) const;

private:
	std::vector<std::unique_ptr<Node>> m_Children;

	Microsoft::WRL::ComPtr<ID3D11Buffer> m_ConstantBuffer;

	DirectX::XMMATRIX m_LocalTransform;
	DirectX::XMMATRIX m_AccumulatedTransform;

	std::string m_NodeName;

	ModelData* m_pModel;
	Node* m_pOwner;
};

#endif
