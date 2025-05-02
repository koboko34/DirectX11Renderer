#pragma once

#ifndef COMMON_H
#define COMMON_H

#define MAX_POINT_LIGHTS 8 // if changing this number, remember to also update it in the pixel shader
#define MAX_DIRECTIONAL_LIGHTS 1
#define MAX_PLANE_CHUNKS 1024
#define MAX_INSTANCE_COUNT 1024

#include <vector>
#include <utility>
#include <string>

#include "DirectXMath.h"

struct Vertex
{
	DirectX::XMFLOAT3 Pos;
	DirectX::XMFLOAT3 Normal;
	DirectX::XMFLOAT2 TexCoord;
};

typedef unsigned long long UINT64;

struct RenderStats
{
	std::vector<std::pair<std::string, UINT64>> TrianglesRendered;
	std::vector<std::pair<std::string, UINT64>> InstancesRendered;
	double FrameTime;
	double FPS;
};

#endif
