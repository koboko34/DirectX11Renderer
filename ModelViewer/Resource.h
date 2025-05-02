#pragma once

#ifndef RESOURCE_H
#define RESOURCE_H

#include <string>

typedef unsigned int UINT;

class Resource
{
	friend class ResourceManager;

public:
	Resource(void* pData) : m_pData(pData) { AddRef(); }

	UINT AddRef() { return ++m_RefCount; }
	UINT RemoveRef();

	void* GetDataPtr() const { return m_pData; }

private:
	UINT m_RefCount = 0;
	void* m_pData = nullptr;

};

#endif
