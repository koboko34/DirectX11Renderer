#include "ResourceManager.h"

#include <cassert>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "Application.h"

ResourceManager* ResourceManager::ms_Instance = nullptr;

ResourceManager* ResourceManager::GetSingletonPtr()
{
	if (!ResourceManager::ms_Instance)
	{
		ResourceManager::ms_Instance = new ResourceManager();
	}
	return ResourceManager::ms_Instance;
}

void* ResourceManager::LoadResource(const std::string& Filepath)
{
	// check if it already loaded, if so, add to ref count and return the data ptr
	auto itRes = m_ResourceMap.find(Filepath);
	if (itRes != m_ResourceMap.end())
	{
		itRes->second->AddRef();
		return itRes->second->m_pData;
	}

	// else try to load resource
	size_t Pos = Filepath.find('.');
	std::string Extension;
	if (Pos == std::string::npos)
	{
		assert(false && "Invalid filepath!");
	}
	Extension = Filepath.substr(Pos);

	void* pData = nullptr;
	auto it = m_Loaders.find(Extension);
	if (it != m_Loaders.end())
	{
		pData = it->second(Filepath);
	}
	else
	{
		assert(false && "Unknown file extension!");
	}

	if (!pData)
	{
		return nullptr;
	}

	m_ResourceMap[Filepath] = std::make_unique<Resource>(pData, Filepath, Extension);
	return pData;
}

bool ResourceManager::UnloadResource(const std::string& Filepath)
{
	// returns true if the resource's ref count is still above 0 after decrementing
	
	Resource* ResourceToUnload = m_ResourceMap[Filepath].get();
	if (!ResourceToUnload)
	{
		return false;
	}

	if (ResourceToUnload->RemoveRef() < 1)
	{
		auto it = m_Unloaders.find(ResourceToUnload->m_Extension);
		assert(it != m_Unloaders.end() && "Failed to find unloader by extension!");
		it->second(Filepath);
		m_ResourceMap.erase(Filepath);
		return false;
	}
	return true;
}

ID3D11ShaderResourceView* ResourceManager::LoadTexture(const char* Filepath)
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

	assert(FAILED(Application::GetSingletonPtr()->GetGraphics()->GetDevice()->CreateTexture2D(&TexDesc, &InitData, &Texture)) == false);
	assert(FAILED(Application::GetSingletonPtr()->GetGraphics()->GetDevice()->CreateShaderResourceView(Texture, NULL, &TextureView)) == false);

	if (NeedsAlpha)
	{
		delete[] ImageDataRgba;
	}
	stbi_image_free(ImageData);

	return TextureView;
}

void* ResourceManager::LoadPng(const std::string& Filepath)
{
	return LoadTexture(Filepath.c_str());
}

void* ResourceManager::LoadJpg(const std::string& Filepath)
{
	return LoadTexture(Filepath.c_str());
}

void ResourceManager::UnloadPng(const std::string& Filepath)
{
	return;
}

void ResourceManager::UnloadJpg(const std::string& Filepath)
{
	return;
}
