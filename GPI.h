#pragma once

#include "GPIPipeline.h"
#include "GPIResource.h"

struct IGPIPipeline;
struct GPIResourceDesc;
struct GPIPipelineStateDesc;
struct GPIRenderTargetViewDesc;
struct GPIDepthStencilViewDesc;
struct GPIConstantBufferViewDesc;
struct GPIShaderResourceViewDesc;
struct GPIUnorderedAccessViewDesc;
struct GPISamplerDesc;

/* GPU Interface */
/* GAI Graphics Adaptor Interface? */
class IGPI
{
public:
	IGPI() = default;
	virtual ~IGPI() = default;

	virtual void Initialize() abstract;
	virtual void BeginFrame( const IGPIResource& inSwapChainResource, const IGPIRenderTargetView& inSwapChainRTV, const IGPIDepthStencilView& inSwapChainDSV ) abstract;
	virtual void EndFrame( const IGPIResource& inSwapChainResource ) abstract;

	virtual void ClearSwapChain( IGPIRenderTargetView* inRTV ) abstract;
	virtual void ClearRenderTarget( IGPIRenderTargetView* inRTV ) abstract;

	virtual void SetPipelineState( const GPIPipelineStateDesc& pipelineDesc ) abstract;
	virtual void Render( const GPIPipelineInput& pipelineInput ) abstract;
	virtual void ExecuteCommandList() abstract;

	virtual uint32 GetSwapChainCurrentIndex() abstract;
	virtual IGPIResourceRef GetSwapChainResource( const uint32 index ) abstract;

	virtual IGPIPipelineRef CreatePipelineState( const GPIPipelineStateDesc& pipelineDesc ) abstract;

	virtual IGPIResourceRef CreateResource( const GPIResourceDesc& desc ) abstract;
	virtual IGPIResourceRef CreateResource( const GPIResourceDesc& desc, void* data, uint32 sizeInBytes ) abstract;

	virtual IGPIRenderTargetViewRef CreateRenderTargetView( const IGPIResource& inResource, const GPIRenderTargetViewDesc& rtvDesc ) abstract;
	virtual IGPIDepthStencilViewRef CreateDepthStencilView( const IGPIResource& inResource, const GPIDepthStencilViewDesc& dsvDesc ) abstract;
	virtual IGPIConstantBufferViewRef CreateConstantBufferView( const IGPIResource& inResource, const GPIConstantBufferViewDesc& cbvDesc ) abstract;
	virtual IGPIShaderResourceViewRef CreateShaderResourceView( const IGPIResource& inResource, const GPIShaderResourceViewDesc& srvDesc ) abstract;
	virtual IGPIUnorderedAccessViewRef CreateUnorderedAccessView( const IGPIResource& inResource, const GPIUnorderedAccessViewDesc& uavDesc ) abstract;
	virtual IGPISamplerRef CreateSampler( const IGPIResource& inResource, const GPISamplerDesc& samplerDesc ) abstract;
	virtual IGPIVertexBufferViewRef CreateVertexBufferView( const IGPIResource& inResource, const uint32 size, const uint32 stride ) abstract;
	virtual IGPIIndexBufferViewRef CreateIndexBufferView( const IGPIResource& inResource, const uint32 size ) abstract;

	virtual void BindRenderTargetView( IGPIPipeline& inPipeline, const IGPIRenderTargetView& inRTV, uint32 index ) abstract;
	virtual void BindConstantBufferView( IGPIPipeline& inPipeline, const IGPIConstantBufferView& inCBV, uint32 index ) abstract;
	virtual void BindShaderResourceView( IGPIPipeline& inPipeline, const IGPIShaderResourceView& inSRV, uint32 index ) abstract;
	virtual void BindUnorderedAccessView( IGPIPipeline& inPipeline, const IGPIUnorderedAccessView& inUAV, uint32 index ) abstract;
	virtual void BindDepthStencilView( IGPIPipeline& inPipeline, const IGPIDepthStencilView& inDSV ) abstract;

	virtual void UpdateResourceData( const IGPIResource& inResource, void* data, uint32 sizeInBytes ) abstract;

	//virtual void UpdateConstantBuffer( uint32 bufferID, void* data, uint32 size ) abstract;

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