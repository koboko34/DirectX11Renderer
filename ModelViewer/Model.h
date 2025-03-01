#pragma once

#ifndef MODEL_H
#define MODEL_H

#include <unordered_map>

#include <d3d11.h>
#include <DirectXMath.h>

#include <wrl.h>

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"


class Model
{
private:
	struct Vertex
	{
		DirectX::XMFLOAT3 Pos;
		DirectX::XMFLOAT3 Normal;
		DirectX::XMFLOAT2 TexCoord;
	};

	struct Mesh
	{
		unsigned int VerticesOffset;
		unsigned int IndicesOffset;
		unsigned int VertexCount;
		unsigned int IndexCount;
		unsigned int MaterialIndex;
	};

	struct Material
	{
		struct MaterialData
		{
			DirectX::XMFLOAT3 DiffuseColor = { 0.f, 0.f, 0.f };
			int DiffuseSRV = -1;
		};
		
		DirectX::XMFLOAT3 DiffuseColor;
		int DiffuseSRV = -1;
		Microsoft::WRL::ComPtr<ID3D11Buffer> ConstantBuffer;
	};

public:
	Model();
	Model(const Model& Other);
	~Model();

	bool Initialise(ID3D11Device* Device, ID3D11DeviceContext* DeviceContext, std::string ModelFilename, std::string TexturesPath);
	void Shutdown();
	void Render(ID3D11DeviceContext* DeviceContext);

private:
	void ShutdownBuffers();
	void RenderMeshes(ID3D11DeviceContext* DeviceContext);

	bool LoadModel();
	void ReleaseModel();
	void Reset();

	void ProcessNode(aiNode* Node, const aiScene* Scene);
	void ProcessMesh(aiMesh* Mesh, const aiScene* Scene);

	bool CreateBuffers();
	void LoadMaterials(const aiScene* Scene);
	void LoadTexture(aiMaterial* ModelMaterial, unsigned int Index, Material& Mat);
	void CreateConstantBuffer(Material& Mat);

private:
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_VertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_IndexBuffer;
	std::vector<Vertex> m_Vertices;
	std::vector<unsigned int> m_Indices;
	std::vector<Mesh> m_Meshes;
	std::vector<Material> m_Materials;
	std::unordered_map<unsigned int, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> m_TexturesMap;

	std::string m_ModelPath;
	std::string m_TexturesPath;
};

#endif
