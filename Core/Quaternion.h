#pragma once

#include "Vector.h"
#include "Matrix.h"

#include <DirectXMath.h>

// @TODO: Remove AI generated codes, XM functions
struct Quaternion
{
    Quaternion()
        : _q( DirectX::XMQuaternionIdentity() )
    {}

    explicit Quaternion( DirectX::XMVECTOR q )
        : _q( q )
    {}

    Quaternion( float x, float y, float z, float w )
        : _q( DirectX::XMVectorSet( x, y, z, w ) )
    {}

    // (x, y, z, w)
    DirectX::XMVECTOR _q;

    // 항상 정규화된 quaternion로 사용
    Quaternion normalized() const
    {
        return Quaternion( DirectX::XMQuaternionNormalize( _q ) );
    }

    // ----------------------------
    // 3x3 회전 행렬 (XMMATRIX)
    // ----------------------------
    // 상단 3x3만 의미 있음
    DirectX::XMMATRIX toMatrix3x3() const
    {
        // XMMatrixRotationQuaternion은
        // [ R 0 ]
        // [ 0 1 ] 형태의 4x4 반환
        return DirectX::XMMatrixRotationQuaternion(
            DirectX::XMQuaternionNormalize( _q )
        );
    }

    // ----------------------------
    // 벡터 회전 (translation 없음)
    // ----------------------------
    DirectX::XMVECTOR rotateVector( DirectX::XMVECTOR v ) const
    {
        // v는 (x,y,z,0) 또는 (x,y,z,1) 상관없음
        // 내부적으로 quaternion * v * q^-1
        return DirectX::XMVector3Rotate( v, _q );
    }

    // ----------------------------
    // 회전 결합 (this * rhs)
    // ----------------------------
    Quaternion operator*( const Quaternion& rhs ) const
    {
        return Quaternion(
            DirectX::XMQuaternionMultiply( _q, rhs._q )
        );
    }

    // ----------------------------
    // Axis-Angle 생성 (rad)
    // ----------------------------
    static Quaternion fromAxisAngle( DirectX::XMVECTOR axis, float angleRad )
    {
        return Quaternion(
            DirectX::XMQuaternionRotationAxis( axis, angleRad )
        );
    }
};
