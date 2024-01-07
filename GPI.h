#pragma once

#include "GPIResource.h"

struct GPIPipelineStateDesc;

/* GPU Interface */
class IGPI
{
public:
	IGPI() = default;
	virtual ~IGPI() = default;

	virtual void Initialize() abstract;
	virtual void BeginFrame() abstract;
	virtual void EndFrame() abstract;

	virtual void SetPipelineState( const GPIPipelineStateDesc& pipelineDesc ) abstract;
	virtual void Render( IVertexBuffer* positionBuffer, IVertexBuffer* uvBuffer, IVertexBuffer* normalBuffer, IIndexBuffer* indexBuffer ) abstract;
	virtual void FlushPipelineState() abstract;

	virtual IVertexBufferRef CreateVertexBuffer( void* data, uint32 stride, uint32 size ) abstract;
	virtual IIndexBufferRef CreateIndexBuffer( void* data, uint32 size ) abstract;

	virtual void CreatePipelineState( const GPIPipelineStateDesc& pipelineStateDesc ) abstract;

	virtual void UpdateConstantBuffer( const struct ConstantBuffer& constBuffer ) abstract;

	virtual void RunCS() abstract;

	void SetWindowSize( uint32 width, uint32 height )
	{
		_windowWidth = width;
		_windowHeight = height;
	}

protected:
	uint32 _windowWidth;
	uint32 _windowHeight;
};