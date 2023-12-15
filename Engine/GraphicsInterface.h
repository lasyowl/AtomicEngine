#pragma once

#include "EngineEssential.h"
#include "RenderResource.h"
#include "PipelineStateObject.h"

struct SGraphicsInitParam
{
	SRect WindowRect;
};

/** Interface of graphics API */
class IGraphicsInterface
{
public:
	virtual void InitGraphics( const SGraphicsInitParam& InParam ) abstract;
	virtual void BeginFrame() abstract;
	virtual void EndFrame() abstract;

	virtual void SetPipelineState( IPipelineStateRef& InPipelineState ) abstract;
	virtual void Render( const class CRenderObject& InRenderObject ) abstract;
	virtual void FlushPipelineState() abstract;

public:
	virtual IPipelineStateRef CreatePipelineState() abstract;

	virtual IVertexBufferContextRef CreateVertexBuffer( void* Data, uint64 NumBytes ) abstract;
	virtual IIndexBufferContextRef CreateIndexBuffer( void* Data, uint64 NumBytes ) abstract;
};