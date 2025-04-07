#pragma once

#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include <unordered_map>
#include <string>
#include <memory>
#include <functional>
#include <array>

#include "d3d11.h"

#include "Resource.h"

enum ResourceType
{
	Texture2D,
	SRV,

	Default
};

class ResourceManager
{
private:
	ResourceManager() {}

	static ResourceManager* ms_Instance;

public:
	static ResourceManager* GetSingletonPtr();

	void* LoadResource(const std::string& Filepath, ResourceType Type = Default);
	bool UnloadResource(const std::string& Filepath, ResourceType Type = Default);

	std::unordered_map<std::string, std::array<std::unique_ptr<Resource>, ResourceType::Default + 1>>& GetResourceMap() { return m_ResourceMap; }

private:
	static void* LoadPng(const std::string& Filepath, ResourceType Type);
	static void* LoadJpg(const std::string& Filepath, ResourceType Type);

	static void UnloadPng(const std::string& Filepath);
	static void UnloadJpg(const std::string& Filepath);

	static ID3D11ShaderResourceView* LoadTexture(const char* Filepath);
	static ID3D11Texture2D* LoadAsTexture2D(const char* Filepath);

private:
	std::unordered_map<std::string, std::array<std::unique_ptr<Resource>, ResourceType::Default + 1>> m_ResourceMap;

	const std::unordered_map<std::string, std::function<void*(const std::string&, ResourceType)>> m_Loaders = {
		{".png",  LoadPng},
		{".jpg",  LoadJpg},
		{".jpeg", LoadJpg},
	};

	const std::unordered_map<std::string, std::function<void (const std::string&)>> m_Unloaders = {
		{".png",  UnloadPng},
		{".jpg",  UnloadJpg},
		{".jpeg", UnloadJpg},
	};

};

#endif
