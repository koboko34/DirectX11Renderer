#pragma once

#ifndef GAME_OBJECT_H
#define GAME_OBJECT_H

#include "Model.h"
#include "Component.h"

class GameObject : public Component
{
public:
	GameObject();

	void SetName(const std::string& NewName) { m_Name = NewName; }

	void SendTransformToModels();

	virtual void AddComponent(std::shared_ptr<Component> Comp) override;

	size_t GetUID() const { return m_UID; }
	const std::string& GetName() const { return m_Name; }

	const std::vector<Model*>& GetModels() const { return m_Models; }

private:
	std::string m_Name;
	size_t m_UID;

	std::vector<Model*> m_Models;

	static size_t ms_UID;
};

#endif
