#include "ResourceManager.h"

#include <cassert>
#include <algorithm>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "Graphics.h"

ResourceManager* ResourceManager::ms_Instance = nullptr;

ResourceManager* ResourceManager::GetSingletonPtr()
{
	if (!ResourceManager::ms_Instance)
	{
		ResourceManager::ms_Instance = new ResourceManager();
	}
	return ResourceManager::ms_Instance;
}

void ResourceManager::Shutdown()
{
	m_TexturesMap.clear();
}

ID3D11ShaderResourceView* ResourceManager::LoadTexture(const std::string& Filepath)
{
	auto itRes = m_TexturesMap.find(Filepath);
	if (itRes != m_TexturesMap.end() && itRes->second.get())
	{
		itRes->second->AddRef();
		return reinterpret_cast<ID3D11ShaderResourceView*>(itRes->second->m_pData);
	}

	ID3D11ShaderResourceView* pData = Internal_LoadTexture(Filepath.c_str());
	if (!pData)
	{
		return nullptr;
	}

	m_TexturesMap[Filepath] = std::make_unique<Resource>(pData, Filepath);
	return pData;
}

bool ResourceManager::UnloadTexture(const std::string& Filepath)
{
	// returns true if the resource's ref count is still above 0 after decrementing
	
	Resource* ResourceToUnload = m_TexturesMap[Filepath].get();
	if (!ResourceToUnload)
	{
		return false;
	}

	ResourceToUnload->RemoveRef();
	if (ResourceToUnload->m_RefCount > 0)
	{
		return true;
	}

	Internal_UnloadTexture(Filepath);
	return false;
}

ID3D11ShaderResourceView* ResourceManager::Internal_LoadTexture(const char* Filepath)
{
	int Width, Height, Channels;
	unsigned char* ImageData = stbi_load(Filepath, &Width, &Height, &Channels, 0);
	assert(ImageData);

	unsigned char* ImageDataRgba = ImageData;
	bool NeedsAlpha = Channels == 3;

	ID3D11Texture2D* Texture;
	ID3D11ShaderResourceView* TextureView = nullptr;

	D3D11_TEXTURE2D_DESC TexDesc = {};
	TexDesc.Width = Width;
	TexDesc.Height = Height;
	TexDesc.MipLevels = 1;
	TexDesc.ArraySize = 1;
	TexDesc.SampleDesc.Count = 1;
	TexDesc.Usage = D3D11_USAGE_IMMUTABLE;
	TexDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	if (Channels == 1)
	{
		TexDesc.Format = DXGI_FORMAT_R8_UNORM;
	}
	else if (Channels == 2)
	{
		TexDesc.Format = DXGI_FORMAT_R8G8_UNORM;
	}
	else if (Channels == 3)
	{
		ImageDataRgba = new unsigned char[Width * Height * 4];

		for (int i = 0; i < Width * Height; i++)
		{
			ImageDataRgba[i * 4 + 0] = ImageData[i * 3 + 0];
			ImageDataRgba[i * 4 + 1] = ImageData[i * 3 + 1];
			ImageDataRgba[i * 4 + 2] = ImageData[i * 3 + 2];
			ImageDataRgba[i * 4 + 3] = 255;
		}

		Channels = 4;
		TexDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	}
	else
	{
		TexDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	}

	D3D11_SUBRESOURCE_DATA InitData = {};
	InitData.pSysMem = ImageDataRgba;
	InitData.SysMemPitch = Width * Channels;

	assert(FAILED(Graphics::GetSingletonPtr()->GetDevice()->CreateTexture2D(&TexDesc, &InitData, &Texture)) == false);
	assert(FAILED(Graphics::GetSingletonPtr()->GetDevice()->CreateShaderResourceView(Texture, NULL, &TextureView)) == false);

	if (NeedsAlpha)
	{
		delete[] ImageDataRgba;
	}
	stbi_image_free(ImageData);

	TextureView->SetPrivateData(WKPDID_D3DDebugObjectName, (UINT)strlen(Filepath), Filepath);
	Texture->Release();

	return TextureView;
}

void ResourceManager::Internal_UnloadTexture(const std::string& Filepath)
{
	ID3D11ShaderResourceView* SRV = reinterpret_cast<ID3D11ShaderResourceView*>(m_TexturesMap[Filepath]->m_pData);
	SRV->Release();
	m_TexturesMap.erase(Filepath);
}
