#pragma once

#include "EngineEssential.h"
#include "Vector.h"

struct StaticMesh
{
public:
	StaticMesh() {}

	Vec3* GetPositionPtr() { return position.data(); }
	Vec2* GetUVPtr() { return uv.data(); }
	uint32* GetIndexPtr( uint32 meshIndex ) { return indices[ meshIndex ].data(); }

	uint32 GetNumPosition() { return position.size(); }
	uint32 GetNumUV() { return uv.size(); }
	uint32 GetNumMeshes() { return indices.size(); }
	uint32 GetNumIndices( uint32 meshIndex ) { return indices[ meshIndex ].size(); }

	uint32 GetPositionStride() { return sizeof( Vec3 ); }
	uint32 GetUVStride() { return sizeof( Vec2 ); }

	uint32 GetPositionByteSize() { return GetNumPosition() * sizeof( Vec3 ); }
	uint32 GetUVByteSize() { return GetNumUV() * sizeof( Vec2 ); }
	uint32 GetIndexByteSize( uint32 meshIndex ) { return GetNumIndices( meshIndex ) * sizeof( uint32 ); }

public:
	std::string name;
	std::vector<Vec3> position;
	std::vector<Vec3> normal;
	std::vector<Vec2> uv;
	std::vector<std::vector<uint32>> indices;
};