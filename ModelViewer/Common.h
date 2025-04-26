#pragma once

#ifndef COMMON_H
#define COMMON_H

#define MAX_POINT_LIGHTS 8 // if changing this number, remember to also update it in the pixel shader
#define MAX_DIRECTIONAL_LIGHTS 1
#define MAX_PLANE_CHUNKS 1024

#include "DirectXMath.h"

struct Vertex
{
	DirectX::XMFLOAT3 Pos;
	DirectX::XMFLOAT3 Normal;
	DirectX::XMFLOAT2 TexCoord;
};

#endif
