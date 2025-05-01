#pragma once

#ifndef AABB_H
#define AABB_H

#include <cfloat>
#include <vector>

#include "DirectXMath.h"

struct AABB
{
	DirectX::XMFLOAT3 Min = {  FLT_MAX,  FLT_MAX,  FLT_MAX };
	DirectX::XMFLOAT3 Max = { -FLT_MAX, -FLT_MAX, -FLT_MAX };

	std::vector<DirectX::XMFLOAT4> Corners;

	void Expand(DirectX::XMFLOAT3 Pos);
	void CalcCorners();
};

#endif
