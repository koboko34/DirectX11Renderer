#pragma once

#ifndef SHADER_RESOURCE_H
#define SHADER_RESOURCE_H

#include <memory>

#include "d3d11.h"

#include "wrl.h"

#include "Resource.h"

struct ShaderResource
{	
	std::unique_ptr<Resource> ShaderRes;
	Microsoft::WRL::ComPtr<ID3D10Blob> Bytecode;
};

#endif
