#pragma once

#include "EngineEssential.h"
#include "RenderResource.h"

class CRenderObject
{
public:
	void InitRenderResource();

	const CVertexBuffer& GetVertexBuffer() const { return VertexBuffer; }
	const CIndexBuffer& GetIndexBuffer() const { return IndexBuffer; }

private:
	CVertexBuffer VertexBuffer;
	CIndexBuffer IndexBuffer;
};

typedef std::shared_ptr<CRenderObject> CRenderObjectRef;