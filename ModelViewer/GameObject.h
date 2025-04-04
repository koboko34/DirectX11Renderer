#pragma once

#ifndef GAME_OBJECT_H
#define GAME_OBJECT_H

#include "DirectXMath.h"

#include "Model.h"
#include "Component.h"

struct Transform
{
	DirectX::XMFLOAT3 Position  = { 0.f, 0.f, 0.f };
	DirectX::XMFLOAT3 Rotation  = { 0.f, 0.f, 0.f };
	DirectX::XMFLOAT3 Scale		= { 1.f, 1.f, 1.f };
};

class GameObject
{
public:
	GameObject();

	void Shutdown();

	void SetPosition(float x, float y, float z);
	void SetRotation(float x, float y, float z);
	void SetScale(float x, float y, float z);
	void SetTransform(const Transform& NewTransform);

	void AddComponent(Component* Comp);

	void SendTransformToModels();

	const std::vector<std::shared_ptr<Component>>& GetComponents() const { return m_Components; }
	const std::vector<std::shared_ptr<Model>>& GetModels() const { return m_Models; }

	const DirectX::XMMATRIX GetWorldMatrix() const;

	const DirectX::XMFLOAT3 GetPosition() const { return m_Transform.Position; }
	const DirectX::XMFLOAT3 GetRotation() const { return m_Transform.Rotation; }
	const DirectX::XMFLOAT3 GetScale() const { return m_Transform.Scale; }

	DirectX::XMFLOAT3* GetPositionPtr() { return &m_Transform.Position; }

private:
	Model* m_Model = nullptr;
	Transform m_Transform;

	std::vector<std::shared_ptr<Component>> m_Components;
	std::vector<std::shared_ptr<Model>> m_Models;

	size_t m_UID;

	static size_t ms_UID;
};

#endif
