#pragma once

#include "GPIResource.h"

struct GPIPipelineStateDesc;

/* GPU Interface */
/* GAI Graphics Adaptor Interface? */
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

	virtual IVertexBufferRef CreateResourceBuffer( void* data, uint32 size ) abstract;

	virtual void CreatePipelineState( const GPIPipelineStateDesc& pipelineDesc ) abstract;

	virtual void BindResourceBuffer( const GPIPipelineStateDesc& pipelineDesc, IVertexBuffer* resourceBuffer, uint32 index ) abstract;

	virtual void UpdateConstantBuffer( uint32 bufferHash, void* data, uint32 size ) abstract;
	virtual void UpdateConstantBuffer1( const GPIPipelineStateDesc& pipelineDesc, void* data ) abstract;

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