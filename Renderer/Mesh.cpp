#include "Mesh.h"

#include <Core/Cache.h>
#include <RenderBackend/RB.h>
#include <RenderBackend/RBUtility.h>
#include <Renderer/SampleMesh.h>
#include <Renderer/Material.h>

std::unordered_map<std::string, MeshRef> MeshCache::_cache;
std::unordered_map<std::string, MeshRef> MeshCache::_cache2;

void MeshCache::BuildSamples( IRenderBackend& rb )
{
    //BuildMesh( rb, SampleMesh::GetCube() );
    //BuildMesh( rb, SampleMesh::GetQuad() );
    //BuildMesh( rb, SampleMesh::GetPlane() );
    BuildMesh( rb, SampleMesh::GetSphere() );
}

MeshRef MeshCache::BuildMesh( IRenderBackend& rb, const MeshData& meshData )
{
    MeshRef mesh = std::make_shared<Mesh>();
    mesh->data = meshData;

    RBResourceDesc desc = RBUtil::GetVertexResourceDesc( "", 0 );

    desc.name = meshData.name + "_Vertex";
    desc.width = meshData.GetVertexByteSize();
    mesh->vertexBuffer.CreateRenderResources( rb, meshData.GetVertexPtr(), meshData.GetVertexByteSize(), meshData.vertexStride );

    for ( uint32 index = 0; index < 1/*meshData.GetNumMeshes()*/; ++index )
    {
        desc.name = meshData.name + "_Index";
        desc.width = meshData.GetIndexByteSize();
        mesh->indexBuffer.CreateRenderResources( rb, meshData.GetIndexPtr(), meshData.GetIndexByteSize() );
    }

    _cache[ meshData.name ] = mesh;

    return mesh;
}

MeshRef& MeshCache::Add( const std::string& name, const MeshData& meshData )
{
    assert( !_cache.contains( name ) );

    //_cache[ name ] = BuildMesh( meshData );
    //_cache2[ name ] = BuildMesh( meshData );

    return _cache[ name ];
}

MeshRef& MeshCache::Find( const std::string& name )
{/*
    static bool bInit = true;
    if ( bInit )
    {
        _cache[ "cube" ] = BuildMesh( SampleMesh::GetCube() );
        _cache[ "quad" ] = BuildMesh( SampleMesh::GetQuad() );
        _cache[ "plane" ] = BuildMesh( SampleMesh::GetPlane() );
        _cache[ "sphere" ] = BuildMesh( SampleMesh::GetSphere() );

        bInit = false;
    }*/

    assert( _cache.contains( name ) );

    return _cache[ name ];
}

const MeshRef& MeshCache::Find2( const std::string& name )
{
    static bool bInit = true;
    if ( bInit )
    {/*
        _cache2[ "cube" ] = SampleMesh::GetCube();
        _cache2[ "quad" ] = SampleMesh::GetQuad();
        _cache2[ "plane" ] = SampleMesh::GetPlane();
        _cache2[ "sphere" ] = SampleMesh::GetSphere();*/

        bInit = false;
    }

    assert( _cache2.contains( name ) );

    return _cache2[ name ];
}

MeshEntity::MeshEntity( std::shared_ptr<MeshData> meshData, const Vec3& position )
    : IEntity()
    , _meshData( meshData )
    , _position( position )
{
    _material = TCache<Material>::GetInstance().Find( meshData->materialName );
}

void MeshEntity::SetPosition( const Vec3& position )
{
    _position = position;
}

void MeshEntity::CreateRenderResources( IRenderBackend& rb )
{
    _mesh = MeshCache::BuildMesh( rb, *_meshData );

    // @TODO: Clean up duplication with UpdateConstantBuffer
    __declspec( align( 256 ) )
        struct ConstantBufferData
    {
        Mat4x4 model;
        Mat4x4 rotation;
    } cbData;
    cbData.model = Mat4x4::identity;
    cbData.rotation = Mat4x4::identity;

    _constantBuffer.CreateRenderResources( rb, &cbData, sizeof( ConstantBufferData ) );

    // @TODO: move to better place
    if ( !_material->textureViewTable )
    {
        _material->CreateRenderResources( rb );
    }
}

void MeshEntity::CollectRenderBatches()
{
    // Implementation for collecting mesh render batches
}

void MeshEntity::UpdateConstantBuffer( IRenderBackend& rb )
{
    // @TODO: Clean up duplication with CreateRenderResources
    __declspec( align( 256 ) )
        struct ConstantBufferData
    {
        Mat4x4 model;
        Mat4x4 rotation;
    } cbData;
    cbData.model = Mat4x4::identity;
    cbData.model.mat.r[ 0 ].m128_f32[ 3 ] = GetPosition().x;
    cbData.model.mat.r[ 1 ].m128_f32[ 3 ] = GetPosition().y;
    cbData.model.mat.r[ 2 ].m128_f32[ 3 ] = GetPosition().z;
    cbData.rotation = Mat4x4::identity;

    rb.UpdateResourceData( *_constantBuffer.GetResource(), &cbData, sizeof( ConstantBufferData ) );
}
