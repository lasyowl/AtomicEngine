#pragma once

#include <Core/IntVector.h>
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

	virtual void ClearSwapChain( const IGPIRenderTargetView& inRTV ) abstract;
	virtual void ClearRenderTarget( const IGPIRenderTargetView& inRTV ) abstract;
	virtual void ClearRenderTarget( const IGPIUnorderedAccessView& inUAV ) abstract;

	virtual void SetPipelineState( const GPIPipelineStateDesc& pipelineDesc ) abstract;
	virtual void Render( const GPIPipelineInput& pipelineInput ) abstract;
	virtual void ExecuteCommandList() abstract;

	virtual uint32 GetSwapChainCurrentIndex() abstract;
	virtual IGPIResourceRef GetSwapChainResource( const uint32 index ) abstract;

	virtual IGPIPipelineRef CreatePipelineState( const GPIPipelineStateDesc& pipelineDesc ) abstract;

	virtual IGPIResourceRef CreateResource( const GPIResourceDesc& desc ) abstract;
	virtual IGPIResourceRef CreateResource( const GPIResourceDesc& desc, const void* data, uint32 sizeInBytes ) abstract;
	virtual IGPIRayTraceBottomLevelASRef CreateRayTraceBottomLevelAS( const GPIRayTraceBottomLevelASDesc& asDesc, const IGPIVertexBufferView& inVBV, const IGPIIndexBufferView& inIBV ) abstract;
	virtual IGPIRayTraceTopLevelASRef CreateRayTraceTopLevelAS( const std::vector<IGPIRayTraceBottomLevelASRef>& inBottomLevelAS, const GPIRayTraceTopLevelASDesc& asDesc ) abstract;

	virtual IGPIRenderTargetViewRef CreateRenderTargetView( const IGPIResource& inResource, const GPIRenderTargetViewDesc& rtvDesc ) abstract;
	virtual IGPIDepthStencilViewRef CreateDepthStencilView( const IGPIResource& inResource, const GPIDepthStencilViewDesc& dsvDesc ) abstract;
	virtual IGPIConstantBufferViewRef CreateConstantBufferView( const IGPIResource& inResource, const GPIConstantBufferViewDesc& cbvDesc ) abstract;
	virtual IGPIShaderResourceViewRef CreateShaderResourceView( const IGPIResource& inResource, const GPIShaderResourceViewDesc& srvDesc ) abstract;
	virtual IGPIUnorderedAccessViewRef CreateUnorderedAccessView( const IGPIResource& inResource, const GPIUnorderedAccessViewDesc& uavDesc, const bool bShaderHidden ) abstract;
	virtual IGPITextureViewTableRef CreateTextureViewTable( const std::vector<const IGPIResource*> inResources, const std::vector<GPIShaderResourceViewDesc> inDescs ) abstract;
	virtual IGPISamplerRef CreateSampler( const IGPIResource& inResource, const GPISamplerDesc& samplerDesc ) abstract;
	virtual IGPIVertexBufferViewRef CreateVertexBufferView( const IGPIResource& inResource, const uint32 size, const uint32 stride ) abstract;
	virtual IGPIIndexBufferViewRef CreateIndexBufferView( const IGPIResource& inResource, const uint32 size ) abstract;

	virtual void CopyBufferRegion( const IGPIResource& inCopyDst, const IGPIResource& inCopySrc, const uint32 copyDstByteOffset, const uint32 copyByteSize ) abstract;

	virtual void BindRenderTargetView( IGPIPipeline& inPipeline, const IGPIRenderTargetView& inRTV, uint32 index ) abstract;
	virtual void BindConstantBufferView( IGPIPipeline& inPipeline, const IGPIConstantBufferView& inCBV, uint32 index ) abstract;
	virtual void BindShaderResourceView( IGPIPipeline& inPipeline, const IGPIShaderResourceView& inSRV, uint32 index ) abstract;
	virtual void BindUnorderedAccessView( IGPIPipeline& inPipeline, const IGPIUnorderedAccessView& inUAV, uint32 index ) abstract;
	virtual void BindDepthStencilView( IGPIPipeline& inPipeline, const IGPIDepthStencilView& inDSV ) abstract;
	virtual void BindTextureViewTable( IGPIPipeline& inPipeline, const IGPITextureViewTable& inTable, const uint32 index ) abstract;

	virtual void UpdateResourceData( const IGPIResource& inResource, void* data, uint32 sizeInBytes ) abstract;

	virtual void TransitionResource( const IGPIResource& inResource, const EGPIResourceStates statesBefore, const EGPIResourceStates statesAfter ) abstract;

	virtual void RunCS() abstract;

	virtual void RayTrace( const GPIPipelineStateDesc& desc, const IGPIRayTraceTopLevelASRef& inRTRAS, IGPIShaderResourceViewRef testNormalSRV, IGPIShaderResourceViewRef testIndexSRV, IGPIShaderResourceViewRef testIndexOffsetSRV ) abstract;

	void SetWindowSize( const IVec2& size ) { _windowSize = size; }
	IVec2 GetWindowSize() { return _windowSize; }

protected:
	IVec2 _windowSize;
};