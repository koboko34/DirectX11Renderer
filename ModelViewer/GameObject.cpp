#include "GameObject.h"
#include "ImGui/imgui.h"

size_t GameObject::ms_UID = 0;

GameObject::GameObject()
{
	m_UID = GameObject::ms_UID++;
	m_Name = "GameObject_" + std::to_string(m_UID);
	m_ComponentName = "Game Object";
}

void GameObject::RenderControls()
{
	ImGui::Text(m_ComponentName.c_str());
	ImGui::InputFloat3("Position", reinterpret_cast<float*>(&m_Transform.Position));
	ImGui::InputFloat3("Rotation", reinterpret_cast<float*>(&m_Transform.Rotation));
	ImGui::InputFloat3("Scale", reinterpret_cast<float*>(&m_Transform.Scale));
}
