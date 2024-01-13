#pragma once

#include "ECSDefine.h"
#include "GPIResource.h"
#include "StaticMesh.h"

struct PrimitiveComponent
{
	DEFINE_ECS_COMPONENT( Primitive );

	std::shared_ptr<StaticMesh> staticMesh;

	IVertexBufferRef positionBuffer;
	IVertexBufferRef normalBuffer;
	IVertexBufferRef uvBuffer;
	std::vector<IIndexBufferRef> indexBuffer;
};
