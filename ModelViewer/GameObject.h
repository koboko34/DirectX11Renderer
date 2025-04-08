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
	void SetName(const std::string& NewName) { m_Name = NewName; }

	void AddComponent(std::shared_ptr<Component> Comp);

	void SendTransformToModels();

	size_t GetUID() const { return m_UID; }
	const std::string& GetName() const { return m_Name; }

	const std::vector<std::shared_ptr<Component>>& GetComponents() const { return m_Components; }
	const std::vector<Model*>& GetModels() const { return m_Models; }

	const DirectX::XMMATRIX GetWorldMatrix() const;

	const DirectX::XMFLOAT3 GetPosition() const { return m_Transform.Position; }
	const DirectX::XMFLOAT3 GetRotation() const { return m_Transform.Rotation; }
	const DirectX::XMFLOAT3 GetScale() const { return m_Transform.Scale; }

	DirectX::XMFLOAT3* GetPositionPtr() { return &m_Transform.Position; }

private:
	std::string m_Name;
	size_t m_UID;

	Transform m_Transform;

	std::vector<std::shared_ptr<Component>> m_Components;
	std::vector<Model*> m_Models;

	static size_t ms_UID;
};

#endif
