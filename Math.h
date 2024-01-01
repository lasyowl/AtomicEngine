#pragma once

#include "Matrix.h"

namespace AEMath
{
Mat4x4 GetViewMatrix( Vec3 origin, Vec3 target, Vec3 up )
{
	Mat4x4 result;
	result.mat = DirectX::XMMatrixLookAtLH( DirectX::XMVECTOR{ origin.x, origin.y, origin.z, 0 },
											DirectX::XMVECTOR{ target.x, target.y, target.z, 0 },
											DirectX::XMVECTOR{ up.x, up.y, up.z, 0 } );
	result.mat = DirectX::XMMatrixTranspose( result.mat );
	return result;
}

Mat4x4 GetPerspectiveMatrix( float viewWidth, float viewHeight, float nearZ, float farZ )
{
	Mat4x4 result;
	result.mat = DirectX::XMMatrixPerspectiveLH( viewWidth, viewHeight, nearZ, farZ );
	result.mat = DirectX::XMMatrixTranspose( result.mat );
	return result;
}

Mat4x4 GetPerspectiveMatrixFOV( float fovAngleY, float aspectRatio, float nearZ, float farZ )
{
	Mat4x4 result;
	result.mat = DirectX::XMMatrixPerspectiveFovLH( fovAngleY, aspectRatio, nearZ, farZ );
	result.mat = DirectX::XMMatrixTranspose( result.mat );
	return result;
}
}