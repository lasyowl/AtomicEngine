#pragma once

#include "ECSDefine.h"
#include "GPIResource.h"
#include "GPIPipeline.h"
#include "StaticMesh.h"

struct PrimitiveComponent
{
	DEFINE_ECS_COMPONENT( Primitive );

	StaticMeshRef staticMesh;
};
