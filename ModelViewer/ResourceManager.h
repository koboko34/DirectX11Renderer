#pragma once

#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include <unordered_map>
#include <string>
#include <memory>

#include "d3d11.h"

#include "Resource.h"

class ModelData;

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
	ModelData* LoadModel(const std::string& ModelPath, const std::string& TexturesPath);

	UINT UnloadTexture(const std::string& Filepath);
	UINT UnloadModel(const std::string& Filepath);

	std::unordered_map<std::string, std::unique_ptr<Resource>>& GetTexturesMap() { return m_TexturesMap; }
	std::unordered_map<std::string, std::unique_ptr<Resource>>& GetModelsMap() { return m_ModelsMap; }

private:
	ID3D11ShaderResourceView* Internal_LoadTexture(const char* Filepath);
	ModelData* Internal_LoadModel(const char* ModelPath, const char* TexturesPath);

	void Internal_UnloadTexture(const std::string& Filepath);
	void Internal_UnloadModel(const std::string& Filepath);

private:
	std::unordered_map<std::string, std::unique_ptr<Resource>> m_TexturesMap;
	std::unordered_map<std::string, std::unique_ptr<Resource>> m_ModelsMap;

};

#endif
