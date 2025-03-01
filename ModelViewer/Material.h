#pragma once

#ifndef MATERIAL_H
#define MATERIAL_H

#include "DirectXMath.h"

#include "d3d11.h"

#include "wrl.h"

struct aiMaterial;

struct MaterialData
{
	DirectX::XMFLOAT3 DiffuseColor = { 0.f, 0.f, 0.f };
	int DiffuseSRV = -1;
};

class Material
{
	friend class Model;
	friend class Mesh;

public:
	Material(UINT Index, Model* pOwner);

	void LoadTextures(aiMaterial* MeshMat);
	void CreateConstantBuffer();

private:
	DirectX::XMFLOAT3 m_DiffuseColor;
	int m_DiffuseSRV = -1;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_ConstantBuffer;
	bool m_bTwoSided = false;
	
	UINT m_uIndex;
	Model* m_pOwner;

};

#endif
