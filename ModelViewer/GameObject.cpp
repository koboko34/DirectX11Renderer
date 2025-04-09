#include "GameObject.h"

size_t GameObject::ms_UID = 0;

GameObject::GameObject()
{
	m_UID = GameObject::ms_UID++;
	m_Name = "GameObject_" + std::to_string(m_UID);
}
