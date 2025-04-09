#pragma once

#include <vector>
#include <memory>

#include "DirectXMath.h"

class GameObject;

struct Transform
{
	DirectX::XMFLOAT3 Position = { 0.f, 0.f, 0.f };
	DirectX::XMFLOAT3 Rotation = { 0.f, 0.f, 0.f };
	DirectX::XMFLOAT3 Scale = { 1.f, 1.f, 1.f };
};

class Component
{
public:
	virtual ~Component() = default;

	void SetPosition(float x, float y, float z);
	void SetRotation(float x, float y, float z);
	void SetScale(float x, float y, float z);
	void SetTransform(const Transform& NewTransform);

	void SetOwner(Component* pOwner) { m_pOwner = pOwner; }

	virtual void AddComponent(std::shared_ptr<Component> Comp);

	const DirectX::XMFLOAT3 GetPosition() const { return m_Transform.Position; }
	const DirectX::XMFLOAT3 GetRotation() const { return m_Transform.Rotation; }
	const DirectX::XMFLOAT3 GetScale() const { return m_Transform.Scale; }
	const Transform& GetTransform() const { return m_Transform; }
	const DirectX::XMMATRIX GetWorldMatrix() const;
	const DirectX::XMMATRIX GetAccumulatedWorldMatrix() const;

	Component* GetOwner() const { return m_pOwner; }

	const std::vector<std::shared_ptr<Component>>& GetComponents() const { return m_Components; }

protected:
	Component* m_pOwner = nullptr;
	Transform m_Transform;

	std::vector<std::shared_ptr<Component>> m_Components; // I think these should be unique_ptr??? refactor soon

};

