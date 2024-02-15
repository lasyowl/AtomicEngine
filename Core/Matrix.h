#pragma once

#include <DirectXMath.h>

struct Mat4x4
{
	DirectX::XMMATRIX mat;

	static const Mat4x4 identity;

	void Inverse()
	{
		DirectX::XMVECTOR det = DirectX::XMMatrixDeterminant( mat );
		mat = DirectX::XMMatrixInverse( &det, mat );
	}

	friend Mat4x4 operator *(const Mat4x4& lhs, const Mat4x4& rhs)
	{
		Mat4x4 result;
		result.mat = lhs.mat * rhs.mat;
		return result;
	}

	static Mat4x4 Inverse( const Mat4x4& original )
	{
		Mat4x4 result = original;
		result.Inverse();
		return result;
	}
};