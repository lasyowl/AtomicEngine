#pragma once

#include "ECSDefine.h"
#include "StaticMesh.h"

struct PrimitiveComponent
{
	DEFINE_ECS_COMPONENT( Primitive );

	StaticMeshGroupRef staticMeshGroup;
};
