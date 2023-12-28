#pragma once

#include <DirectXMath.h>

struct Vec3
{
	DirectX::XMVECTOR vec;

	static const Vec3 up;
};

const Vec3 Vec3::up = Vec3{ 0,1,0 };