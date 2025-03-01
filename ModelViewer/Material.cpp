#include "Material.h"
#include "Application.h"
#include "MyMacros.h"

#include "assimp/material.h"

Material::Material(UINT Index, Model* pOwner) : m_uIndex(Index), m_pOwner(pOwner)
{
}

void Material::LoadTextures(aiMaterial* MeshMat)
{
	if (m_pOwner->GetTexturesMap().find(m_uIndex) != m_pOwner->GetTexturesMap().end())
	{
		return;
	}

	aiString Path;
	aiColor3D Color;
	if (MeshMat->GetTexture(aiTextureType_DIFFUSE, 0, &Path) == AI_SUCCESS)
	{
		std::string FullPath = m_pOwner->GetTexturesPath() + std::string(Path.C_Str());

		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> SRV = Application::GetSingletonPtr()->GetGraphics()->LoadTexture(FullPath.c_str());
		m_pOwner->GetTexturesMap().insert({m_uIndex, SRV});
		m_DiffuseSRV = (int)m_uIndex;
	}
	else if (MeshMat->Get(AI_MATKEY_COLOR_DIFFUSE, Color) == AI_SUCCESS)
	{
		m_DiffuseColor.x = Color.r;
		m_DiffuseColor.y = Color.g;
		m_DiffuseColor.z = Color.b;
	}
}

void Material::CreateConstantBuffer()
{
	HRESULT hResult;
	D3D11_BUFFER_DESC BufferDesc = {};
	BufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	BufferDesc.ByteWidth = sizeof(MaterialData);
	BufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	MaterialData Data = {};
	Data.DiffuseColor = m_DiffuseColor;
	Data.DiffuseSRV = m_DiffuseSRV;

	D3D11_SUBRESOURCE_DATA BufferData = {};
	BufferData.pSysMem = &Data;

	ASSERT_NOT_FAILED(Application::GetSingletonPtr()->GetGraphics()->GetDevice()->CreateBuffer(&BufferDesc, &BufferData, &m_ConstantBuffer));
}
