#pragma once

#include <Engine/ECSDefine.h>
#include <Engine/StaticMesh.h>
#include <Engine/Material.h>

struct PrimitiveComponent : public IECSComponent
{
	DEFINE_ECS_COMPONENT( Primitive );

	StaticMeshGroupRef staticMeshGroup;

	PBRMaterial material;
};
