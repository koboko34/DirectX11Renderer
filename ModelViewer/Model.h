#pragma once

#ifndef MODEL_H
#define MODEL_H

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

public:
	Model();
	Model(const Model& Other);
	~Model();

	bool Initialise(ID3D11Device* Device, ID3D11DeviceContext* DeviceContext, char* ModelFilename);
	void Shutdown();
	void Render(ID3D11DeviceContext* DeviceContext);

	unsigned int GetIndexCount() const { return m_IndexCount; }

private:
	bool InitialiseBuffers(ID3D11Device* Device);
	void ShutdownBuffers();
	void RenderBuffers(ID3D11DeviceContext* DeviceContext);

	bool LoadModel(char* ModelFilename);
	void ReleaseModel();

private:
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_VertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_IndexBuffer;
	unsigned int m_VertexCount, m_IndexCount;
	std::vector<Vertex> m_Vertices;
	std::vector<unsigned int> m_Indices;
};

#endif
