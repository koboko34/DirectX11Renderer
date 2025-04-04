#pragma once

class GameObject;

class Component
{
public:
	virtual ~Component() = default;

	void SetOwner(GameObject* pOwner) { m_pOwner = pOwner; }
	GameObject* GetOwner() const { return m_pOwner; }

protected:
	GameObject* m_pOwner = nullptr;
};

