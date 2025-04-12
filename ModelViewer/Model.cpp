#include "Model.h"

#include "ResourceManager.h"
#include "ModelData.h"
#include "ImGui/imgui.h"

Model::Model(const std::string& ModelPath, const std::string& TexturesPath)
{
	m_pModelData = ResourceManager::GetSingletonPtr()->LoadModel(ModelPath, TexturesPath);
	assert(m_pModelData);

	m_ComponentName = "Model";
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

void Model::RenderControls()
{
	ImGui::Text(m_ComponentName.c_str());
	ImGui::Checkbox("Should Render?", &m_bActive);
}

void Model::SendTransformToModel()
{
	m_pModelData->GetTransforms().push_back(DirectX::XMMatrixTranspose(GetAccumulatedWorldMatrix()));
}
