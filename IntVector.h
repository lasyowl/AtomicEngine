#pragma once

#include "EngineDefine.h"

#include <DirectXMath.h>

struct IVec3
{
	IVec3() : x( 0 ), y( 0 ), z( 0 ) {}
	IVec3( int32 inX, int32 inY, int32 inZ ) : x( inX ), y( inY ), z( inZ ) {}

	int32 x;
	int32 y;
	int32 z;

	static const IVec3 up;

	IVec3& operator *= ( const int32 coef )
	{
		x *= coef;
		y *= coef;
		z *= coef;

		return *this;
	}

	friend IVec3 operator +( const IVec3& lhs, const IVec3& rhs )
	{
		return IVec3( lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z );
	}
};
