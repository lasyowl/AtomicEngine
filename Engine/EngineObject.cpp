#include "EngineObject.h"

#include <Engine/KeyboardInput.h>
#include <Engine/MouseInput.h>

#include <Renderer/Mesh.h>
#include <Renderer/View.h>
#include <Renderer/RenderCommand.h>

///////////////////////////////
//// SceneObject
///////////////////////////////
SceneObject::SceneObject( const Vec3& position )
    : _bRenderStateDirty( true )
    , _position( position )
    , _front( Vec3::front )
    , _right( Vec3::right )
    , _up( Vec3::up )
{}

void SceneObject::SetPosition( const Vec3& position )
{
    if ( _position != position )
    {
        _position = position;
        MarkRenderStateDirty();
    }
}

void SceneObject::Move( const Vec3& delta )
{
    SetPosition( _position + delta );
}

void SceneObject::SetRotation( const Quaternion& rotation )
{
    _rotation = rotation;

    DirectX::XMVECTOR front = DirectX::XMVector3Rotate( DirectX::XMVectorSet( 0, 0, 1, 0 ), rotation._q );
    DirectX::XMVECTOR right = DirectX::XMVector3Rotate( DirectX::XMVectorSet( 1, 0, 0, 0 ), rotation._q );
    _front = Vec3( DirectX::XMVectorGetX( front ), DirectX::XMVectorGetY( front ), DirectX::XMVectorGetZ( front ) );
    _right = Vec3( DirectX::XMVectorGetX( right ), DirectX::XMVectorGetY( right ), DirectX::XMVectorGetZ( right ) );

    MarkRenderStateDirty();
}

///////////////////////////////
//// ViewObject
///////////////////////////////
ViewObject::ViewObject( const Vec3& position )
    : SceneObject( position )
    , _entity( nullptr )
{}

void ViewObject::Tick()
{
    if ( IsRenderStateDirty() )
    {
        RenderUtil::EnqueueRenderCommand( [ entity = _entity, position = GetPosition(), rotation = GetRotation() ]( IRenderBackend& rb )
        {
            entity->SetPosition( position );
            entity->SetRotation( rotation );
            entity->UpdateViewProjectionMatrix( rb );
        } );

        MarkRenderStateDirty( false );
    }
}

IEntity* ViewObject::CreateEntity()
{
    _entity = new ViewEntity( GetPosition(), GetRotation() );

    RenderUtil::EnqueueRenderCommand( [ entity = _entity ]( IRenderBackend& rb )
    {
        entity->CreateRenderResources( rb );
    } );

    return _entity;
}

///////////////////////////////
//// MeshObject
///////////////////////////////
void MeshObject::Tick()
{
    // @TODO: Remove test code
    //SetPosition( GetPosition() + Vec3( 0.001f, 0.0f, 0.0f ) );
    SetPosition( Vec3( 0.0f, 0.0f, 0.0f ) );

    if ( IsRenderStateDirty() )
    {
        RenderUtil::EnqueueRenderCommand( [ entity = _entity, position = GetPosition() ]( IRenderBackend& rb )
        {
            entity->SetPosition( position );
            entity->UpdateConstantBuffer( rb );
        } );

        MarkRenderStateDirty( false );
    }
}

IEntity* MeshObject::CreateEntity()
{
    _entity = new MeshEntity( _meshData, GetPosition());

    RenderUtil::EnqueueRenderCommand( [ entity = _entity ]( IRenderBackend& rb )
    {
        entity->CreateRenderResources( rb );
    } );

    return _entity;
}
