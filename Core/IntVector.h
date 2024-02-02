#pragma once

#include "Core.h"

struct IVec2
{
public:
	IVec2() : x( 0 ), y( 0 ) {}
	IVec2( int32 inX, int32 inY ) : x( inX ), y( inY ) {}

public:
	int32 x;
	int32 y;

public:
	IVec2& operator *= ( const int32 coef )
	{
		x *= coef;
		y *= coef;

		return *this;
	}

	friend IVec2 operator +( const IVec2& lhs, const IVec2& rhs )
	{
		return IVec2( lhs.x + rhs.x, lhs.y + rhs.y );
	}
};

struct IVec3
{
public:
	IVec3() : x( 0 ), y( 0 ), z( 0 ) {}
	IVec3( int32 inX, int32 inY, int32 inZ ) : x( inX ), y( inY ), z( inZ ) {}

public:
	int32 x;
	int32 y;
	int32 z;

public:
	static const IVec3 up;

public:
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
