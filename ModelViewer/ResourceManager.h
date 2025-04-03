#pragma once

#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include <unordered_map>
#include <string>
#include <memory>
#include <functional>

#include "d3d11.h"

#include "Resource.h"

class ResourceManager
{
private:
	ResourceManager() {}

	static ResourceManager* ms_Instance;

public:
	static ResourceManager* GetSingletonPtr();

	void* LoadResource(const std::string& Filepath);
	bool UnloadResource(const std::string& Filepath);

	std::unordered_map<std::string, std::unique_ptr<Resource>>& GetResourceMap() { return m_ResourceMap; }

private:
	static void* LoadPng(const std::string& Filepath);
	static void* LoadJpg(const std::string& Filepath);

	static void UnloadPng(const std::string& Filepath);
	static void UnloadJpg(const std::string& Filepath);

	static ID3D11ShaderResourceView* LoadTexture(const char* Filepath);

private:
	std::unordered_map<std::string, std::unique_ptr<Resource>> m_ResourceMap;

	const std::unordered_map<std::string, std::function<void*(const std::string&)>> m_Loaders = {
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
