#pragma once

#include <DirectXMath.h>

struct Mat4x4
{
	DirectX::XMMATRIX mat;

	Mat4x4& operator *(const Mat4x4& other)
	{
		mat = mat * other.mat;
		return *this;
	}
};