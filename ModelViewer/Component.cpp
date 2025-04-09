#include "Component.h"

void Component::SetPosition(float x, float y, float z)
{
	m_Transform.Position = DirectX::XMFLOAT3(x, y, z);
}

void Component::SetRotation(float x, float y, float z)
{
	m_Transform.Rotation = DirectX::XMFLOAT3(x, y, z);
}

void Component::SetScale(float x, float y, float z)
{
	m_Transform.Scale = DirectX::XMFLOAT3(x, y, z);
}

void Component::SetTransform(const Transform& NewTransform)
{
	m_Transform = NewTransform;
}

void Component::AddComponent(std::shared_ptr<Component> Comp)
{
	if (!Comp.get())
	{
		return;
	}

	Comp->SetOwner(this);
	m_Components.push_back(Comp);
}

const DirectX::XMMATRIX Component::GetWorldMatrix() const
{
	DirectX::XMMATRIX Matrix = DirectX::XMMatrixIdentity();
	Matrix *= DirectX::XMMatrixScaling(m_Transform.Scale.x, m_Transform.Scale.y, m_Transform.Scale.z);
	Matrix *= DirectX::XMMatrixRotationY(DirectX::XMConvertToRadians(m_Transform.Rotation.y));
	Matrix *= DirectX::XMMatrixRotationX(DirectX::XMConvertToRadians(m_Transform.Rotation.x));
	Matrix *= DirectX::XMMatrixRotationZ(DirectX::XMConvertToRadians(m_Transform.Rotation.z));
	Matrix *= DirectX::XMMatrixTranslation(m_Transform.Position.x, m_Transform.Position.y, m_Transform.Position.z);

	return Matrix;
}

const DirectX::XMMATRIX Component::GetAccumulatedWorldMatrix() const
{
	if (m_pOwner == nullptr)
	{
		return GetWorldMatrix();
	}

	DirectX::XMMATRIX Accumulated = m_pOwner->GetAccumulatedWorldMatrix();
	Accumulated *= GetWorldMatrix();

	return Accumulated;
}
