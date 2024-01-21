#include "stdafx.h"
#include "StaticMesh.h"
#include "AtomicEngine.h"
#include "GPI.h"
#include "SampleMesh.h"

std::unordered_map<std::string, StaticMeshRef> StaticMeshCache::_cache;

StaticMeshRef BuildStaticMesh( StaticMeshData meshData )
{
	StaticMeshRef mesh = std::make_shared<StaticMesh>();

	GPIResourceDesc desc{};
	desc.dimension = EGPIResourceDimension::Buffer;
	desc.format = EGPIResourceFormat::Unknown;
	desc.height = 1;
	desc.depth = 1;
	desc.numMips = 1;
	desc.flags = GPIResourceFlag_None;

	mesh->pipelineInput.vbv.resize( 3 );

	desc.width = meshData.GetPositionByteSize();
	mesh->positionResource = AtomicEngine::GetGPI()->CreateResource( desc, meshData.GetPositionPtr(), meshData.GetPositionByteSize() );
	mesh->pipelineInput.vbv[ 0 ] = AtomicEngine::GetGPI()->CreateVertexBufferView( *mesh->positionResource, meshData.GetPositionByteSize(), meshData.GetPositionStride() );

	if( !meshData.normal.empty() )
	{
		desc.width = meshData.GetNormalByteSize();
		mesh->normalResource = AtomicEngine::GetGPI()->CreateResource( desc, meshData.GetNormalPtr(), meshData.GetNormalByteSize() );
		mesh->pipelineInput.vbv[ 1 ] = AtomicEngine::GetGPI()->CreateVertexBufferView( *mesh->normalResource, meshData.GetNormalByteSize(), meshData.GetNormalStride() );
	}
	if( !meshData.uv.empty() )
	{
		desc.width = meshData.GetUVByteSize();
		mesh->uvResource = AtomicEngine::GetGPI()->CreateResource( desc, meshData.GetUVPtr(), meshData.GetUVByteSize() );
		mesh->pipelineInput.vbv[ 2 ] = AtomicEngine::GetGPI()->CreateVertexBufferView( *mesh->uvResource, meshData.GetUVByteSize(), meshData.GetUVStride() );
	}

	for( uint32 index = 0; index < meshData.GetNumMeshes(); ++index )
	{
		desc.width = meshData.GetIndexByteSize( index );
		IGPIResourceRef ib = AtomicEngine::GetGPI()->CreateResource( desc, meshData.GetIndexPtr( index ), meshData.GetIndexByteSize( index ) );
		IGPIIndexBufferViewRef ibv = AtomicEngine::GetGPI()->CreateIndexBufferView( *ib, meshData.GetIndexByteSize( index ) );
		mesh->indexResource.emplace_back( ib );
		mesh->pipelineInput.ibv.emplace_back( ibv );
	}

	return mesh;
}

StaticMeshRef& StaticMeshCache::AddStaticMesh( const std::string& name, const StaticMeshData& meshData )
{
	assert( !_cache.contains( name ) );

	_cache[ name ] = BuildStaticMesh( meshData );

	return _cache[ name ];
}

StaticMeshRef& StaticMeshCache::FindStaticMesh( const std::string& name )
{
	static bool bInit = true;
	if( bInit )
	{
		_cache[ "cube" ] = BuildStaticMesh( SampleMesh::GetCube() );
		_cache[ "quad" ] = BuildStaticMesh( SampleMesh::GetQuad() );
		_cache[ "sphere" ] = BuildStaticMesh( SampleMesh::GetSphere() );

		bInit = false;
	}

	assert( _cache.contains( name ) );

	return _cache[ name ];
}