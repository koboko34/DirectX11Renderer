#include "GameObject.h"

size_t GameObject::ms_UID = 0;

GameObject::GameObject()
{
	m_UID = GameObject::ms_UID++;
	m_Name = "GameObject_" + std::to_string(m_UID);
}

void GameObject::Shutdown()
{
}

void GameObject::AddComponent(std::shared_ptr<Component> Comp)
{
	Component::AddComponent(Comp);

	if (Model* m = dynamic_cast<Model*>(Comp.get()))
	{
		m_Models.push_back(m);
	}
}

void GameObject::SendTransformToModels()
{
	for (Model* m : m_Models)
	{
		//m->GetTransforms().push_back(DirectX::XMMatrixTranspose(m->GetAccumulatedWorldMatrix())); // once I add models to resource manager, we can start using this
		m->GetTransforms().push_back(DirectX::XMMatrixTranspose(GetWorldMatrix()));
	}

	// might want to loop here through Component GameObjects and recurse
}
