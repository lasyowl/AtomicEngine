#pragma once

#include <Core/IntVector.h>
#include <Renderer/Entity.h>

class Material;

enum EMeshAttribute : uint8
{
    MA_Position    = 1 << 0,
    MA_Normal      = 1 << 1,
    MA_UV          = 1 << 2,
    MA_Tangent     = 1 << 3,
    MA_Color       = 1 << 4,
};
inline EMeshAttribute operator | ( EMeshAttribute lhs, EMeshAttribute rhs )
{
    return static_cast<EMeshAttribute>( (uint8)lhs | (uint8)rhs );
}

struct MeshData
{
public:
    const uint8* GetVertexPtr() const { return vertices.data(); }
    const IVec3* GetIndexPtr() const { return indices[ 0 ].data(); }

    uint32 GetNumIndices() const { return 3 * indices[ 0 ].size(); }

    uint32 GetVertexByteSize() const { return vertices.size(); }
    uint32 GetIndexByteSize() const { return indices[ 0 ].size() * sizeof( IVec3 ); }

public:
    std::string name;

    EMeshAttribute attributeFlag;

    uint8 vertexStride;
    uint8 positionOffset;
    uint8 normalOffset;
    uint8 uvOffset;

    std::vector<uint8> vertices;
    // @TODO: Disassemble IVec3 to uint32 array for index buffer
    std::vector<std::vector<IVec3>> indices;

    std::string materialName;
};

// Non-interleaved version
//struct MeshData
//{
//public:
//    const Vec3* GetPositionPtr() const { return position.data(); }
//    const Vec3* GetNormalPtr() const { return normal.data(); }
//    const Vec2* GetUVPtr() const { return uv.data(); }
//    const IVec3* GetIndexPtr( uint32 meshIndex ) const { return indices[ meshIndex ].data(); }
//
//    uint32 GetNumPosition() const { return position.size(); }
//    uint32 GetNumNormal() const { return normal.size(); }
//    uint32 GetNumUV() const { return uv.size(); }
//    uint32 GetNumMeshes() const { return indices.size(); }
//    uint32 GetNumIndices( uint32 meshIndex ) const { return 3 * indices[ meshIndex ].size(); }
//
//    uint32 GetPositionStride() const { return sizeof( Vec3 ); }
//    uint32 GetNormalStride() const { return sizeof( Vec3 ); }
//    uint32 GetUVStride() const { return sizeof( Vec2 ); }
//
//    uint32 GetPositionByteSize() const { return GetNumPosition() * sizeof( Vec3 ); }
//    uint32 GetNormalByteSize() const { return GetNumNormal() * sizeof( Vec3 ); }
//    uint32 GetUVByteSize() const { return GetNumUV() * sizeof( Vec2 ); }
//    uint32 GetIndexByteSize( uint32 meshIndex ) const { return GetNumIndices( meshIndex ) * sizeof( IVec3 ); }
//
//public:
//    std::string name;
//    EMeshAttribute attributeFlag;
//
//    std::vector<uint8> vertices;
//    std::vector<Vec3> position;
//    std::vector<Vec3> normal;
//    std::vector<Vec2> uv;
//    std::vector<std::vector<IVec3>> indices;
//};

struct Mesh
{
    MeshData data;

    VertexBuffer vertexBuffer;
    IndexBuffer indexBuffer;
};
using MeshRef = std::shared_ptr<Mesh>;

class MeshCache
{
public:
    static void BuildSamples( IRenderBackend& rb );
    static MeshRef BuildMesh( IRenderBackend& rb, const MeshData& meshData );
    static MeshRef& Add( const std::string& name, const MeshData& meshData );
    static MeshRef& Find( const std::string& name );
    static const MeshRef& Find2( const std::string& name );

private:
    static std::unordered_map<std::string, MeshRef> _cache;
    static std::unordered_map<std::string, MeshRef> _cache2;
};

class MeshEntity : public IEntity
{
public:
    MeshEntity( std::shared_ptr<MeshData> meshData, const Vec3& position );

public:
    virtual void CreateRenderResources( IRenderBackend& rb ) override;

    virtual void CollectRenderBatches();

    virtual const MeshData& GetMeshData() const
    {
        return _mesh->data;
    }

    virtual IRBVertexBufferView* GetVertexBufferView() const
    {
        return _mesh->vertexBuffer.GetView();
    }

    virtual IRBIndexBufferView* GetIndexBufferView() const
    {
        return _mesh->indexBuffer.GetView();
    }

    virtual IRBConstantBufferView* GetConstantBufferView() const
    {
        return _constantBuffer.GetView();
    }

    virtual void UpdateConstantBuffer( IRenderBackend& rb );

    Material* GetMaterial() const { return _material.get(); }

    void SetPosition( const Vec3& position );
    const Vec3& GetPosition() const { return _position; }

private:
    Vec3 _position;

    // @TODO: Remove mesh data pointer
    std::shared_ptr<MeshData> _meshData;
    MeshRef _mesh;

    std::shared_ptr<Material> _material;

    // @TODO: Make shared among multiple render objects
    ConstantBuffer _constantBuffer;
};
