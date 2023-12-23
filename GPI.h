#pragma once

class IGPI
{
public:
	IGPI() = default;
	virtual ~IGPI() = default;

	virtual void Initialize() abstract;
	virtual void BeginFrame() abstract;
	virtual void EndFrame() abstract;

	virtual void SetPipelineState( IPipelineStateRef& InPipelineState ) abstract;
	virtual void Render( const class CRenderObject& InRenderObject ) abstract;
	virtual void FlushPipelineState() abstract;
};