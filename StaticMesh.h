#pragma once

#include "EngineEssential.h"
#include "Vector.h"
#include "IntVector.h"
#include "GPIResource.h"
#include "GPIPipeline.h"

struct StaticMeshData
{
public:
	StaticMeshData() {}

	Vec3* GetPositionPtr() { return position.data(); }
	Vec3* GetNormalPtr() { return normal.data(); }
	Vec2* GetUVPtr() { return uv.data(); }
	IVec3* GetIndexPtr( uint32 meshIndex ) { return indices[ meshIndex ].data(); }

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

struct StaticMesh
{
	IGPIResourceRef positionResource;
	IGPIResourceRef normalResource;
	IGPIResourceRef uvResource;
	std::vector<IGPIResourceRef> indexResource;

	GPIPipelineInput pipelineInput;
};

using StaticMeshRef = std::shared_ptr<StaticMesh>;

class StaticMeshCache
{
public:
	static StaticMeshRef& AddStaticMesh( const std::string& name, const StaticMeshData& meshData );

	static StaticMeshRef& FindStaticMesh( const std::string& name );

private:
	static std::unordered_map<std::string, StaticMeshRef> _cache;
};