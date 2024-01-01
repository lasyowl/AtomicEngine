#pragma once

#include "GPIResource.h"
#include "StaticMesh.h"

struct PrimitiveComponent
{
	std::shared_ptr<StaticMesh> staticMesh;

	IVertexBufferRef positionBuffer;
	IVertexBufferRef uvBuffer;
	std::vector<IIndexBufferRef> indexBuffer;
};
