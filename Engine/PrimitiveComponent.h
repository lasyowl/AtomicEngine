#pragma once

#include <Engine/ECSDefine.h>
#include <Engine/StaticMesh.h>
#include <Engine/Material.h>

struct PrimitiveComponent
{
	DEFINE_ECS_COMPONENT( Primitive );

	StaticMeshGroupRef staticMeshGroup;

	PBRMaterialSimple material;
};
