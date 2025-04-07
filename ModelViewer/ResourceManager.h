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

class ResourceManager
{
private:
	ResourceManager() {}

	static ResourceManager* ms_Instance;

public:
	static ResourceManager* GetSingletonPtr();

	void Shutdown();

	// these must NOT be stored with a ComPtr and should be unloaded using UnloadTexture when no longer needed
	ID3D11ShaderResourceView* LoadTexture(const std::string& Filepath);
	bool UnloadTexture(const std::string& Filepath);

	std::unordered_map<std::string, std::unique_ptr<Resource>>& GetTexturesMap() { return m_TexturesMap; }

private:
	ID3D11ShaderResourceView* Internal_LoadTexture(const char* Filepath);
	void Internal_UnloadTexture(const std::string& Filepath);

private:
	std::unordered_map<std::string, std::unique_ptr<Resource>> m_TexturesMap;

};

#endif
