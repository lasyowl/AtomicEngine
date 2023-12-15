#include "stdafx.h"
#include "Renderer.h"
#include "GraphicsInterface.h"
#include "RenderObjectBatch.h"

extern std::shared_ptr<IGraphicsInterface> gGraphics;

void CRenderer::Render( const CRenderObjectBatch& RenderObjectBatch )
{
	IPipelineStateRef PipelineState = RenderObjectBatch.GetPipelineState();
	gGraphics->SetPipelineState( PipelineState );

	for ( const CRenderObjectRef& RenderObject : RenderObjectBatch.GetRenderObjects() )
	{
		gGraphics->Render( *RenderObject );
	}

	gGraphics->FlushPipelineState();
}