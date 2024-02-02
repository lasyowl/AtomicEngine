#pragma once

#include "ECSDefine.h"
#include <GPI/GPIResource.h>
#include <GPI/GPIPipeline.h>
#include "StaticMesh.h"

struct PrimitiveComponent
{
	DEFINE_ECS_COMPONENT( Primitive );

	StaticMeshGroupRef staticMeshGroup;
};
