#pragma once

#include <Core/Vector.h>
#include <Core/IntVector.h>
#include <Core/Quaternion.h>

class IEntity;
class MeshEntity;
class ViewEntity;
struct MeshData;

class IEngineObject
{
public:
    virtual ~IEngineObject() = default;

    virtual void Tick() {}
};

class SceneObject : public IEngineObject
{
protected:
    SceneObject( const Vec3& position );

public:
    virtual ~SceneObject() = default;

    virtual IEntity* CreateEntity() = 0;

public:
    bool IsRenderStateDirty() const { return _bRenderStateDirty; }
    void MarkRenderStateDirty( bool bSet = true ) { _bRenderStateDirty = bSet; }

    const Vec3& GetPosition() const { return _position; }
    void SetPosition( const Vec3& position );
    void Move( const Vec3& delta );

    const Quaternion& GetRotation() const { return _rotation; }
    void SetRotation( const Quaternion& rotation );

    const Vec3& GetFront() const { return _front; }
    const Vec3& GetRight() const { return _right; }
    const Vec3& GetUp() const { return _up; }

private:
    bool _bRenderStateDirty;

    Vec3 _position;
    Vec3 _front;
    Vec3 _right;
    Vec3 _up;
    Quaternion _rotation;
};

class ViewObject : public SceneObject
{
public:
    ViewObject( const Vec3& position );

    virtual void Tick() override;

    virtual IEntity* CreateEntity() override;

private:
    ViewEntity* _entity;
};

class MeshObject : public SceneObject
{
public:
    MeshObject( std::shared_ptr<MeshData> meshData, const Vec3& position )
        : SceneObject( position )
        , _entity( nullptr )
        , _meshData( meshData )
    {}

    virtual void Tick() override;

    virtual IEntity* CreateEntity() override;

private:
    MeshEntity* _entity;

    std::shared_ptr<MeshData> _meshData;
};
