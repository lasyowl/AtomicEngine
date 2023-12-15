#pragma once

#include "EngineEssential.h"
#include "RenderObject.h"
#include "PipelineStateObject.h"

class CRenderObjectBatch
{
public:
	void SetPipelineState( IPipelineStateRef InPipelineState ) { PipelineState = InPipelineState; }
	void AddRenderObject( CRenderObjectRef InRenderObject ) { RenderObjects.push_back( InRenderObject ); }

	IPipelineStateRef GetPipelineState() const { return PipelineState; }
	const std::vector<CRenderObjectRef>& GetRenderObjects() const { return RenderObjects; }

private:
	IPipelineStateRef PipelineState;

	std::vector<CRenderObjectRef> RenderObjects;
};