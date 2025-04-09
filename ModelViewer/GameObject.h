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

	size_t GetUID() const { return m_UID; }
	const std::string& GetName() const { return m_Name; }

private:
	std::string m_Name;
	size_t m_UID;

	static size_t ms_UID;
};

#endif
