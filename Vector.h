#pragma once

#include <DirectXMath.h>

struct Vec2
{
	Vec2() : x( 0 ), y( 0 ) {}
	Vec2( float inX, float inY ) : x( inX ), y( inY ) {}

	float x;
	float y;

	float Magnitude()
	{
		return sqrtf( x * x + y * y );
	}

	Vec2& Normalize()
	{
		float magnitudeInv = 1.0f / Magnitude();
		x *= magnitudeInv;
		y *= magnitudeInv;

		return *this;
	}

	Vec2& operator *= ( const float coef )
	{
		x *= coef;
		y *= coef;

		return *this;
	}

	friend Vec2 operator +( const Vec2& lhs, const Vec2& rhs )
	{
		return Vec2( lhs.x + rhs.x, lhs.y + rhs.y );
	}
};

struct Vec3
{
	Vec3() : x( 0 ), y( 0 ), z( 0 ) {}
	Vec3( float inX, float inY, float inZ ) : x( inX ), y( inY ), z( inZ ) {}

	float x;
	float y;
	float z;

	static const Vec3 up;

	float Magnitude()
	{
		return sqrtf( x * x + y * y + z * z );
	}

	Vec3& Normalize()
	{
		float magnitudeInv = 1.0f / Magnitude();
		x *= magnitudeInv;
		y *= magnitudeInv;
		z *= magnitudeInv;

		return *this;
	}

	Vec3& operator *= ( const float coef )
	{
		x *= coef;
		y *= coef;
		z *= coef;

		return *this;
	}

	friend Vec3 operator +( const Vec3& lhs, const Vec3& rhs )
	{
		return Vec3( lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z );
	}

	static Vec3 Midpoint( const Vec3& lhs, const Vec3& rhs )
	{
		return Vec3( ( lhs.x + rhs.x ) * 0.5f, ( lhs.y + rhs.y ) * 0.5f, ( lhs.z + rhs.z ) * 0.5f );
	}
};

struct Vec4
{
	Vec4() : x( 0 ), y( 0 ), z( 0 ), w( 0 ) {}
	Vec4( float inX, float inY, float inZ, float inW ) : x( inX ), y( inY ), z( inZ ), w( inW ) {}

	float x;
	float y;
	float z;
	float w;

	friend Vec4 operator +( const Vec4& lhs, const Vec4& rhs )
	{
		return Vec4( lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z, lhs.w + rhs.w );
	}
};