#pragma once

#include <Renderer/Entity.h>
#include <Core/Vector.h>
#include <Core/Quaternion.h>

class IRenderBackend;

class ViewEntity : public IEntity
{
public:
    ViewEntity( const Vec3& position, const Quaternion& rotation );

public:
    void SetPosition( const Vec3& position );
    void SetRotation( const Quaternion& position );

    virtual void CreateRenderResources( IRenderBackend& rb ) override;

    void UpdateViewProjectionMatrix( IRenderBackend& rb );

    IRBConstantBufferView* GetConstantBufferView() const { return _constantBuffer.GetView(); }

private:
    Vec3 _position;
    Vec3 _forward;
    Vec3 _up;
    Vec3 _right;

    Quaternion _rotation;

    Mat4x4 _viewMatrix;
    Mat4x4 _projectionMatrix;

    ConstantBuffer _constantBuffer;
};
