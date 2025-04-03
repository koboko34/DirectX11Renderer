#pragma once

#ifndef RESOURCE_H
#define RESOURCE_H

#include <string>

typedef unsigned int UINT;

class Resource
{
	friend class ResourceManager;

public:
	Resource(void* pData, const std::string& Filepath, const std::string& Extension) : m_pData(pData), m_Filepath(Filepath), m_Extension(Extension) { AddRef(); }

	UINT AddRef() { return ++m_RefCount; }
	UINT RemoveRef();

private:
	UINT m_RefCount = 0;
	void* m_pData = nullptr;

	const std::string m_Filepath;
	const std::string m_Extension;
};

#endif
