#include "View.h"

#include <RenderBackend/RB.h>
#include <RenderBackend/RBUtility.h>

ViewEntity::ViewEntity( const Vec3& position, const Quaternion& rotation )
    : IEntity()
    , _position( position )
    , _rotation( rotation )
{

}

void ViewEntity::SetPosition( const Vec3& position )
{
    _position = position;
}

void ViewEntity::SetRotation( const Quaternion& rotation )
{
    _rotation = rotation;
}

void ViewEntity::CreateRenderResources( IRenderBackend& rb )
{
    __declspec( align( 256 ) )
    struct ConstantBufferData
    {
        Mat4x4 viewProjection;
        Mat4x4 viewProjectionInv;
        Vec3 viewPosition;
    } cbData;
    cbData.viewProjection = _viewMatrix * _projectionMatrix;

    RBResourceDesc cbResourceDesc = RBUtil::GetConstantBufferResourceDesc( "ConstantBuffer", sizeof( cbData ) );

    RBConstantBufferViewDesc cbvDesc{};
    cbvDesc.bStatic = false;
    cbvDesc.sizeInBytes = sizeof( cbData );

    _constantBuffer.CreateRenderResources( rb, &cbData, sizeof( cbData ) );
}

void ViewEntity::UpdateViewProjectionMatrix( IRenderBackend& rb )
{
    DirectX::XMVECTOR front = DirectX::XMVector3Rotate( DirectX::XMVectorSet( 0, 0, 1, 0 ), _rotation._q );
    DirectX::XMVECTOR right = DirectX::XMVector3Rotate( DirectX::XMVectorSet( 1, 0, 0, 0 ), _rotation._q );

    _viewMatrix.mat = DirectX::XMMatrixLookAtLH(
        DirectX::XMVectorSet( _position.x, _position.y, _position.z, 1.0f ),
        DirectX::XMVectorSet( _position.x + DirectX::XMVectorGetX( front ), _position.y + DirectX::XMVectorGetY( front ), _position.z + DirectX::XMVectorGetZ( front ), 1.0f ),
        DirectX::XMVectorSet( Vec3::up.x, Vec3::up.y, Vec3::up.z, 0.0f )
    );// *_rotation.toMatrix3x3();

    float fovY = DirectX::XMConvertToRadians( 60.0f );
    float aspect = 1920.0f / 1080.0f;
    float zn = 0.1f;
    float zf = 10000.0f;
    _projectionMatrix.mat = DirectX::XMMatrixPerspectiveFovLH( fovY, aspect, zn, zf );

    __declspec( align( 256 ) )
        struct ConstantBufferData
    {
        Mat4x4 viewProjection;
        Mat4x4 viewProjectionInv;
        Vec3 viewPosition;
    } cbData;

    cbData.viewProjection = _viewMatrix * _projectionMatrix;
    cbData.viewProjection.mat = DirectX::XMMatrixTranspose( cbData.viewProjection.mat );
    cbData.viewPosition = _position;

    rb.UpdateResourceData( *_constantBuffer.GetResource(), &cbData, sizeof( cbData ) );
}
