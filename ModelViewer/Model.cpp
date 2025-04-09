#include "Model.h"

#include "ResourceManager.h"
#include "ModelData.h"

Model::Model(const std::string& ModelPath, const std::string& TexturesPath)
{
	m_pModelData = ResourceManager::GetSingletonPtr()->LoadModel(ModelPath, TexturesPath);
	assert(m_pModelData);
}

Model::~Model()
{
	Shutdown();
}

void Model::Shutdown()
{
	if (!m_pModelData)
	{
		return;
	}

	ResourceManager::GetSingletonPtr()->UnloadModel(m_pModelData->GetModelPath());
	m_pModelData = nullptr;
}

void Model::SendTransformToModel(const DirectX::XMMATRIX& Transform)
{
	m_pModelData->GetTransforms().push_back(Transform);
}
