#include "Math.h"

namespace AEMath
{
Mat4x4 GetTransposedMatrix( const Mat4x4& mat )
{
	Mat4x4 result;
	result.mat = DirectX::XMMatrixTranspose( mat.mat );
	return result;
}

Mat4x4 GetScaleMatrix( const Vec3& scale )
{
	Mat4x4 result;
	result.mat = DirectX::XMMatrixScaling( scale.x, scale.y, scale.z );
	return result;
}

Mat4x4 GetRotationMatrix( const Vec3& rotation )
{
	Mat4x4 result;
	result.mat = DirectX::XMMatrixRotationRollPitchYaw( rotation.x, rotation.y, rotation.z );
	return result;
}

Mat4x4 GetTranslateMatrix( const Vec3& translate )
{
	Mat4x4 result;
	result.mat = DirectX::XMMatrixTranslation( translate.x, translate.y, translate.z );
	return result;
}

Mat4x4 GetViewMatrix( const Vec3& origin, const Vec3& target, const Vec3& up )
{
	Mat4x4 result;
	result.mat = DirectX::XMMatrixLookAtLH( DirectX::XMVECTOR{ origin.x, origin.y, origin.z, 0 },
											DirectX::XMVECTOR{ target.x, target.y, target.z, 0 },
											DirectX::XMVECTOR{ up.x, up.y, up.z, 0 } );
	return result;
}

Mat4x4 GetPerspectiveMatrix( float viewWidth, float viewHeight, float nearZ, float farZ )
{
	Mat4x4 result;
	result.mat = DirectX::XMMatrixPerspectiveLH( viewWidth, viewHeight, nearZ, farZ );
	return result;
}

Mat4x4 GetPerspectiveMatrixFOV( float fovAngleY, float aspectRatio, float nearZ, float farZ )
{
	Mat4x4 result;
	result.mat = DirectX::XMMatrixPerspectiveFovLH( fovAngleY, aspectRatio, nearZ, farZ );
	return result;
}
}