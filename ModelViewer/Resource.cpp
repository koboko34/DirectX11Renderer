#include "Resource.h"

#include <cassert>

#include "ResourceManager.h"

UINT Resource::RemoveRef()
{
	return --m_RefCount;
}
