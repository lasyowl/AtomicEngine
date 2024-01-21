#pragma once

#include "EngineEssential.h"
#include "Vector.h"
#include "IntVector.h"

struct StaticMeshData
{
public:
	StaticMeshData() {}

	Vec3* GetPositionPtr() { return position.data(); }
	Vec3* GetNormalPtr() { return normal.data(); }
	Vec2* GetUVPtr() { return uv.data(); }
	IVec3* GetIndexPtr( uint32 meshIndex ) { return indices[ meshIndex ].data(); }

	uint32 GetNumPosition() { return position.size(); }
	uint32 GetNumNormal() { return normal.size(); }
	uint32 GetNumUV() { return uv.size(); }
	uint32 GetNumMeshes() { return indices.size(); }
	uint32 GetNumIndices( uint32 meshIndex ) { return indices[ meshIndex ].size(); }

	uint32 GetPositionStride() { return sizeof( Vec3 ); }
	uint32 GetNormalStride() { return sizeof( Vec3 ); }
	uint32 GetUVStride() { return sizeof( Vec2 ); }

	uint32 GetPositionByteSize() { return GetNumPosition() * sizeof( Vec3 ); }
	uint32 GetNormalByteSize() { return GetNumNormal() * sizeof( Vec3 ); }
	uint32 GetUVByteSize() { return GetNumUV() * sizeof( Vec2 ); }
	uint32 GetIndexByteSize( uint32 meshIndex ) { return GetNumIndices( meshIndex ) * sizeof( IVec3 ); }

public:
	std::string name;
	std::vector<Vec3> position;
	std::vector<Vec3> normal;
	std::vector<Vec2> uv;
	std::vector<std::vector<IVec3>> indices;
};