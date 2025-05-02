#pragma once

#ifndef MESH_H
#define MESH_H

#include <memory>
#include <string>

#include "DirectXMath.h"
#include "d3d11.h"

#include "wrl.h"

struct aiMesh;
class Material;

class Mesh
{
	friend class ModelData;
	friend class Node;

public:
	Mesh(ModelData* pModel, Node* pNode);

	void ProcessMesh(aiMesh* SceneMesh);

	Microsoft::WRL::ComPtr<ID3D11Buffer> GetArgsBuffer() const { return m_ArgsBuffer; }
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> GetArgsBufferUAV() const { return m_ArgsBufferUAV; }

private:
	bool CreateArgsBuffer();

	void UpdateBoundingBox(DirectX::XMFLOAT3 Pos);

private:
	unsigned int m_VerticesOffset;
	unsigned int m_IndicesOffset;
	unsigned int m_VertexCount;
	unsigned int m_IndexCount;

	Microsoft::WRL::ComPtr<ID3D11Buffer> m_ArgsBuffer;
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> m_ArgsBufferUAV;

	std::shared_ptr<Material> m_Material;
	std::string m_Name;

	ModelData* m_pModel;
	Node* m_pNode;

};

#endif
