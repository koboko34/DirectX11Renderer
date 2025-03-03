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

#include "Node.h"
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
	void Render();

	Microsoft::WRL::ComPtr<ID3D11Buffer> GetVertexBuffer() const { return m_VertexBuffer; }
	Microsoft::WRL::ComPtr<ID3D11Buffer> GetIndexBuffer() const { return m_IndexBuffer; }
	std::vector<Vertex>& GetVertices() { return m_Vertices; }
	std::vector<UINT>& GetIndices() { return m_Indices; }
	std::vector<std::unique_ptr<Mesh>>& GetOpaqueMeshes() { return m_OpaqueMeshes; }
	std::vector<std::unique_ptr<Mesh>>& GetTransparentMeshes() { return m_TransparentMeshes; }
	std::vector<std::shared_ptr<Material>>& GetMaterials() { return m_Materials; }
	std::vector<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>>& GetTextures() { return m_Textures; }
	std::unordered_map<std::string, UINT>& GetTextureIndexMap() { return m_TextureIndexMap; }

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

private:
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_VertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_IndexBuffer;
	std::vector<Vertex> m_Vertices;
	std::vector<UINT> m_Indices;
	std::vector<std::unique_ptr<Mesh>> m_OpaqueMeshes;
	std::vector<std::unique_ptr<Mesh>> m_TransparentMeshes;
	std::unique_ptr<Node> m_RootNode;
	std::vector<std::shared_ptr<Material>> m_Materials;
	std::vector<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> m_Textures;
	std::unordered_map<std::string, UINT> m_TextureIndexMap;

	std::string m_ModelPath;
	std::string m_TexturesPath;
};

#endif
