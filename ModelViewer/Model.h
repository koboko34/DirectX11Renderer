#pragma once

#ifndef MODEL_H
#define MODEL_H

#include <unordered_map>
#include <memory>

#include <d3d11.h>
#include <DirectXMath.h>

#include <wrl.h>

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include "Mesh.h"
#include "Material.h"

struct Vertex
{
	DirectX::XMFLOAT3 Pos;
	DirectX::XMFLOAT3 Normal;
	DirectX::XMFLOAT2 TexCoord;
};

class Model
{
public:
	Model();
	Model(const Model& Other);
	~Model();

	bool Initialise(ID3D11Device* Device, ID3D11DeviceContext* DeviceContext, std::string ModelFilename, std::string TexturesPath);
	void Shutdown();
	void Render(ID3D11DeviceContext* DeviceContext);

	std::vector<Vertex>& GetVertices() { return m_Vertices; }
	std::vector<UINT>& GetIndices() { return m_Indices; }
	std::vector<std::unique_ptr<Mesh>>& GetMeshes() { return m_Meshes; }
	std::vector<std::unique_ptr<Material>>& GetMaterials() { return m_Materials; }
	std::unordered_map<UINT, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>>& GetTexturesMap() { return m_TexturesMap; }

	std::string GetModelPath() const { return m_ModelPath; }
	std::string GetTexturesPath() const { return m_TexturesPath; }

private:
	void ShutdownBuffers();
	void RenderMeshes(ID3D11DeviceContext* DeviceContext);

	bool LoadModel();
	void ReleaseModel();
	void Reset();

	void ProcessNode(aiNode* Node, const aiScene* Scene);

	bool CreateBuffers();
	void LoadMaterials(const aiScene* Scene);

private:
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_VertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_IndexBuffer;
	std::vector<Vertex> m_Vertices;
	std::vector<UINT> m_Indices;
	std::vector<std::unique_ptr<Mesh>> m_Meshes;
	std::vector<std::unique_ptr<Material>> m_Materials;
	std::unordered_map<UINT, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> m_TexturesMap;

	std::string m_ModelPath;
	std::string m_TexturesPath;
};

#endif
