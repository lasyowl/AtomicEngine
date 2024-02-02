#pragma once

#include "Vector.h"
#include "Matrix.h"

namespace AEMath
{
Mat4x4 GetTransposedMatrix( const Mat4x4& mat );
Mat4x4 GetScaleMatrix( const Vec3& scale );
Mat4x4 GetRotationMatrix( const Vec3& rotation );
Mat4x4 GetTranslateMatrix( const Vec3& translate );
Mat4x4 GetViewMatrix( const Vec3& origin, const Vec3& target, const Vec3& up );
Mat4x4 GetPerspectiveMatrix( float viewWidth, float viewHeight, float nearZ, float farZ );
Mat4x4 GetPerspectiveMatrixFOV( float fovAngleY, float aspectRatio, float nearZ, float farZ );
}