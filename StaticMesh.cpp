#include "stdafx.h"
#include "StaticMesh.h"
#include "AtomicEngine.h"
#include "GPI.h"
#include "GPIUtility.h"
#include "SampleMesh.h"

std::unordered_map<std::string, StaticMeshGroupRef> StaticMeshCache::_cache;

StaticMeshGroupRef BuildStaticMeshGroup( const StaticMeshDataGroup& dataGroup )
{
	StaticMeshGroupRef meshGroup = std::make_shared<StaticMeshGroup>();
	meshGroup->meshes.resize( dataGroup.datas.size() );

	GPIResourceDesc desc = GPIUtil::GetVertexResourceDesc( L"", 0 );

	for( uint32 index = 0; index < dataGroup.datas.size(); ++index )
	{
		StaticMesh& mesh = meshGroup->meshes[ index ];
		const StaticMeshData& data = dataGroup.datas[ index ];

		mesh.pipelineInput.vbv.resize( 3 );

		desc.width = data.GetPositionByteSize();
		mesh.positionResource = AtomicEngine::GetGPI()->CreateResource( desc, data.GetPositionPtr(), data.GetPositionByteSize() );
		mesh.pipelineInput.vbv[ 0 ] = AtomicEngine::GetGPI()->CreateVertexBufferView( *mesh.positionResource, data.GetPositionByteSize(), data.GetPositionStride() );

		if( !data.normal.empty() )
		{
			desc.width = data.GetNormalByteSize();
			mesh.normalResource = AtomicEngine::GetGPI()->CreateResource( desc, data.GetNormalPtr(), data.GetNormalByteSize() );
			mesh.pipelineInput.vbv[ 1 ] = AtomicEngine::GetGPI()->CreateVertexBufferView( *mesh.normalResource, data.GetNormalByteSize(), data.GetNormalStride() );
		}
		if( !data.uv.empty() )
		{
			desc.width = data.GetUVByteSize();
			mesh.uvResource = AtomicEngine::GetGPI()->CreateResource( desc, data.GetUVPtr(), data.GetUVByteSize() );
			mesh.pipelineInput.vbv[ 2 ] = AtomicEngine::GetGPI()->CreateVertexBufferView( *mesh.uvResource, data.GetUVByteSize(), data.GetUVStride() );
		}

		for( uint32 index = 0; index < data.GetNumMeshes(); ++index )
		{
			desc.width = data.GetIndexByteSize( index );
			IGPIResourceRef ib = AtomicEngine::GetGPI()->CreateResource( desc, data.GetIndexPtr( index ), data.GetIndexByteSize( index ) );
			IGPIIndexBufferViewRef ibv = AtomicEngine::GetGPI()->CreateIndexBufferView( *ib, data.GetIndexByteSize( index ) );
			mesh.indexResource.emplace_back( ib );
			mesh.pipelineInput.ibv.emplace_back( ibv );
		}
	}

	return meshGroup;
}

StaticMeshGroupRef BuildStaticMeshGroup( StaticMeshData data )
{
	StaticMeshDataGroup group;
	group.datas.push_back( data );

	return BuildStaticMeshGroup( group );
}

StaticMeshGroupRef& StaticMeshCache::AddStaticMeshGroup( const std::string& name, const StaticMeshDataGroup& dataGroup )
{
	assert( !_cache.contains( name ) );

	_cache[ name ] = BuildStaticMeshGroup( dataGroup );

	return _cache[ name ];
}

StaticMeshGroupRef& StaticMeshCache::FindStaticMeshGroup( const std::string& name )
{
	static bool bInit = true;
	if( bInit )
	{
		_cache[ "cube" ] = BuildStaticMeshGroup( SampleMesh::GetCube() );
		_cache[ "quad" ] = BuildStaticMeshGroup( SampleMesh::GetQuad() );
		_cache[ "sphere" ] = BuildStaticMeshGroup( SampleMesh::GetSphere() );

		bInit = false;
	}

	assert( _cache.contains( name ) );

	return _cache[ name ];
}