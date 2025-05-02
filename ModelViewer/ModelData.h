#pragma once

#ifndef MODEL_DATA_H
#define MODEL_DATA_H

#include <string>
#include <memory>
#include <unordered_map>
#include <unordered_set>

#include "wrl.h"

#include "d3d11.h"
#include "DirectXMath.h"

#include "Common.h"
#include "AABB.h"

class Mesh;
class Material;
class Node;
struct aiScene;

class ModelData
{
public:
	ModelData() = delete;
	ModelData(const std::string& ModelPath, const std::string& TexturesPath = "");
	ModelData(const ModelData& Other) = delete;
	~ModelData();

	bool Initialise(ID3D11Device* Device, ID3D11DeviceContext* DeviceContext, const std::string& ModelFilename, const std::string& TexturesPath);
	void Shutdown();
	void Render();

	Microsoft::WRL::ComPtr<ID3D11Buffer> GetVertexBuffer() const { return m_VertexBuffer; }
	Microsoft::WRL::ComPtr<ID3D11Buffer> GetIndexBuffer() const { return m_IndexBuffer; }
	std::vector<Vertex>& GetVertices() { return m_Vertices; }
	std::vector<UINT>& GetIndices() { return m_Indices; }
	std::vector<std::unique_ptr<Mesh>>& GetOpaqueMeshes() { return m_OpaqueMeshes; }
	std::vector<std::unique_ptr<Mesh>>& GetTransparentMeshes() { return m_TransparentMeshes; }
	std::vector<std::shared_ptr<Material>>& GetMaterials() { return m_Materials; }
	std::vector<ID3D11ShaderResourceView*>& GetTextures() { return m_Textures; }
	std::unordered_map<std::string, UINT>& GetTextureIndexMap() { return m_TextureIndexMap; }
	std::unordered_set<std::string>& GetTexturePathsSet() { return m_TexturePathsSet; }

	std::vector<DirectX::XMMATRIX>& GetTransforms() { return m_Transforms; }
	std::vector<DirectX::XMMATRIX>& GetCulledTransforms() { return m_CulledTransforms; }
	AABB& GetBoundingBox() { return m_BoundingBox; }

	std::string GetModelPath() const { return m_ModelPath; }
	std::string GetTexturesPath() const { return m_TexturesPath; }

private:
	void ShutdownBuffers();

	bool LoadModel();
	void ReleaseModel();
	void Reset();

	bool CreateBuffers();
	void LoadMaterials(const aiScene* Scene);

	void RenderMeshes(const std::vector<std::unique_ptr<Mesh>>& Meshes);
	void CopyCulledTransforms();

private:
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_VertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_IndexBuffer;
	std::vector<Vertex> m_Vertices;
	std::vector<UINT> m_Indices;
	std::vector<std::unique_ptr<Mesh>> m_OpaqueMeshes;
	std::vector<std::unique_ptr<Mesh>> m_TransparentMeshes;
	std::unique_ptr<Node> m_RootNode;
	std::vector<std::shared_ptr<Material>> m_Materials;
	std::vector<ID3D11ShaderResourceView*> m_Textures;
	std::unordered_map<std::string, UINT> m_TextureIndexMap;
	std::unordered_set<std::string> m_TexturePathsSet;

	std::vector<DirectX::XMMATRIX> m_Transforms;
	std::vector<DirectX::XMMATRIX> m_CulledTransforms;
	AABB m_BoundingBox;
	
	std::string m_ModelPath;
	std::string m_TexturesPath;

};

#endif
