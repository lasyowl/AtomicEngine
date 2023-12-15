#pragma once
#include "EngineEssential.h"

enum class EPrimitiveTopology
{
	PointList,
	LineList,
	LineListAdjacent,
	TriangleList,
	TriangleListAdjacent,
};

namespace Convert
{
constexpr int32 PrimitiveTopologyToDirectX( EPrimitiveTopology Topology );
}