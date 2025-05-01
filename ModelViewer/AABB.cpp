#include <utility>

#include "AABB.h"

void AABB::Expand(DirectX::XMFLOAT3 Pos)
{
	Min.x = std::min(Min.x, Pos.x);
	Min.y = std::min(Min.y, Pos.y);
	Min.z = std::min(Min.z, Pos.z);
	Max.x = std::max(Max.x, Pos.x);
	Max.y = std::max(Max.y, Pos.y);
	Max.z = std::max(Max.z, Pos.z);
}

void AABB::CalcCorners()
{
	Corners.resize(8);

	int i = 0;
	for (int z = 0; z <= 1; ++z)
	{
		float LocalZ = z == 0 ? Min.z : Max.z;
		for (int y = 0; y <= 1; ++y)
		{
			float LocalY = y == 0 ? Min.y : Max.y;
			for (int x = 0; x <= 1; ++x)
			{
				float LocalX = x == 0 ? Min.x : Max.x;
				Corners[i++] = { LocalX, LocalY, LocalZ, 1.f };
			}
		}
	}
}


