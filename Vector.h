#pragma once

#include <DirectXMath.h>

struct Vec2
{
	float x;
	float y;
};

struct Vec3
{
	Vec3() : x( 0 ), y( 0 ), z( 0 ) {}
	Vec3( float inX, float inY, float inZ ) : x( inX ), y( inY ), z( inZ ) {}

	float x;
	float y;
	float z;

	static const Vec3 up;

	friend Vec3 operator +( const Vec3& lhs, const Vec3& rhs )
	{
		return Vec3( lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z );
	}
};