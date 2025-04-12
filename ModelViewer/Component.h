#pragma once

#include <vector>
#include <memory>
#include <string>

#include "DirectXMath.h"

class Model;

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

	virtual void RenderControls() = 0;

	void SetPosition(float x, float y, float z);
	void SetRotation(float x, float y, float z);
	void SetScale(float x, float y, float z);
	void SetTransform(const Transform& NewTransform);

	void SetOwner(Component* pOwner) { m_pOwner = pOwner; }
	void SetActive(bool bNewActive) { m_bActive = bNewActive; }

	void AddComponent(std::shared_ptr<Component> Comp);

	void SendTransformToModels();

	const DirectX::XMFLOAT3 GetPosition() const { return m_Transform.Position; }
	const DirectX::XMFLOAT3 GetRotation() const { return m_Transform.Rotation; }
	const DirectX::XMFLOAT3 GetScale() const { return m_Transform.Scale; }
	const Transform& GetTransform() const { return m_Transform; }
	Transform& GetTransformRef()  { return m_Transform; }
	const DirectX::XMMATRIX GetWorldMatrix() const;
	const DirectX::XMMATRIX GetAccumulatedWorldMatrix() const;

	Component* GetOwner() const { return m_pOwner; }
	bool IsActive() const { return m_bActive; }

	const std::vector<std::shared_ptr<Component>>& GetComponents() const { return m_Components; }
	const std::vector<Model*>& GetModels() const { return m_Models; }

protected:
	Component* m_pOwner = nullptr;
	Transform m_Transform;
	std::string m_ComponentName = "Component Name";

	std::vector<std::shared_ptr<Component>> m_Components; // I think these should be unique_ptr??? refactor soon
	std::vector<Model*> m_Models;

	bool m_bActive = true;

};

