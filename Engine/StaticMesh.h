#pragma once

#include <Core/Vector.h>
#include <Core/IntVector.h>
#include <GPI/GPIResource.h>
#include <GPI/GPIPipeline.h>

struct StaticMeshData
{
public:
	StaticMeshData() {}

	const Vec3* GetPositionPtr() const { return position.data(); }
	const Vec3* GetNormalPtr() const { return normal.data(); }
	const Vec2* GetUVPtr() const { return uv.data(); }
	const IVec3* GetIndexPtr( uint32 meshIndex ) const { return indices[ meshIndex ].data(); }

	uint32 GetNumPosition() const { return position.size(); }
	uint32 GetNumNormal() const { return normal.size(); }
	uint32 GetNumUV() const { return uv.size(); }
	uint32 GetNumMeshes() const { return indices.size(); }
	uint32 GetNumIndices( uint32 meshIndex ) const { return indices[ meshIndex ].size(); }

	uint32 GetPositionStride() const { return sizeof( Vec3 ); }
	uint32 GetNormalStride() const { return sizeof( Vec3 ); }
	uint32 GetUVStride() const { return sizeof( Vec2 ); }

	uint32 GetPositionByteSize() const { return GetNumPosition() * sizeof( Vec3 ); }
	uint32 GetNormalByteSize() const { return GetNumNormal() * sizeof( Vec3 ); }
	uint32 GetUVByteSize() const { return GetNumUV() * sizeof( Vec2 ); }
	uint32 GetIndexByteSize( uint32 meshIndex ) const { return GetNumIndices( meshIndex ) * sizeof( IVec3 ); }

public:
	std::string name;
	std::vector<Vec3> position;
	std::vector<Vec3> normal;
	std::vector<Vec2> uv;
	std::vector<std::vector<IVec3>> indices;
};

struct StaticMeshDataGroup
{
	std::vector<StaticMeshData> datas;
};

struct StaticMesh
{
	IGPIResourceRef positionResource;
	IGPIResourceRef normalResource;
	IGPIResourceRef uvResource;
	std::vector<IGPIResourceRef> indexResource;

	GPIPipelineInput pipelineInput;
};

struct StaticMeshGroup
{
	std::vector<StaticMesh> meshes;
};

using StaticMeshGroupRef = std::shared_ptr<StaticMeshGroup>;

class StaticMeshCache
{
public:
	static StaticMeshGroupRef& AddStaticMeshGroup( const std::string& name, const StaticMeshDataGroup& dataGroup );
	static StaticMeshGroupRef& FindStaticMeshGroup( const std::string& name );

private:
	static std::unordered_map<std::string, StaticMeshGroupRef> _cache;
};