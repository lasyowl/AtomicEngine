#pragma once

#include "ECSDefine.h"
#include "GPIResource.h"
#include "GPIPipeline.h"
#include "StaticMesh.h"

struct PrimitiveComponent
{
	DEFINE_ECS_COMPONENT( Primitive );

	std::shared_ptr<StaticMeshData> staticMeshData;

	IGPIResourceRef positionResource;
	IGPIResourceRef normalResource;
	IGPIResourceRef uvResource;
	std::vector<IGPIResourceRef> indexResource;

	GPIPipelineInput pipelineInput;
};
