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

void GameObject::SetPosition(float x, float y, float z)
{
	m_Transform.Position = DirectX::XMFLOAT3(x, y, z);
}

void GameObject::SetRotation(float x, float y, float z)
{
	m_Transform.Rotation = DirectX::XMFLOAT3(x, y, z);
}

void GameObject::SetScale(float x, float y, float z)
{
	m_Transform.Scale = DirectX::XMFLOAT3(x, y, z);
}

void GameObject::SetTransform(const Transform& NewTransform)
{
	m_Transform = NewTransform;
}

void GameObject::AddComponent(std::shared_ptr<Component> Comp)
{
	if (!Comp.get())
	{
		return;
	}

	Comp->SetOwner(this);
	m_Components.push_back(Comp);

	if (Model* m = dynamic_cast<Model*>(Comp.get()))
	{
		m_Models.push_back(m);
	}
}

void GameObject::SendTransformToModels()
{
	for (Model* m : m_Models)
	{
		m->GetTransforms().push_back(DirectX::XMMatrixTranspose(GetWorldMatrix()));
	}
}

const DirectX::XMMATRIX GameObject::GetWorldMatrix() const
{
    DirectX::XMMATRIX Matrix = DirectX::XMMatrixIdentity();
	Matrix *= DirectX::XMMatrixScaling(m_Transform.Scale.x, m_Transform.Scale.y, m_Transform.Scale.z);
	Matrix *= DirectX::XMMatrixRotationY(DirectX::XMConvertToRadians(m_Transform.Rotation.y));
	Matrix *= DirectX::XMMatrixRotationX(DirectX::XMConvertToRadians(m_Transform.Rotation.x));
	Matrix *= DirectX::XMMatrixRotationZ(DirectX::XMConvertToRadians(m_Transform.Rotation.z));
	Matrix *= DirectX::XMMatrixTranslation(m_Transform.Position.x, m_Transform.Position.y, m_Transform.Position.z);
    
    return Matrix;
}
