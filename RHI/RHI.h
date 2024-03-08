#pragma once

#include <Core/IntVector.h>
#include "RHIPipeline.h"
#include "RHIResource.h"

struct IRHIPipeline;
struct RHIResourceDesc;
struct RHIPipelineStateDesc;
struct RHIRenderTargetViewDesc;
struct RHIDepthStencilViewDesc;
struct RHIConstantBufferViewDesc;
struct RHIShaderResourceViewDesc;
struct RHIUnorderedAccessViewDesc;
struct RHISamplerDesc;

/* RHI Interface */
class IRHI
{
public:
	IRHI() = default;
	virtual ~IRHI() = default;

	virtual void Initialize() abstract;
	virtual void BeginFrame( const IRHIResource& inSwapChainResource, const IRHIRenderTargetView& inSwapChainRTV, const IRHIDepthStencilView& inSwapChainDSV ) abstract;
	virtual void EndFrame( const IRHIResource& inSwapChainResource ) abstract;

	virtual void ClearSwapChain( const IRHIRenderTargetView& inRTV ) abstract;
	virtual void ClearRenderTarget( const IRHIRenderTargetView& inRTV ) abstract;
	virtual void ClearRenderTarget( const IRHIUnorderedAccessView& inUAV ) abstract;

	virtual void SetPipelineState( const RHIPipelineStateDesc& pipelineDesc ) abstract;
	virtual void Render( const RHIPipelineInput& pipelineInput ) abstract;
	virtual void ExecuteCommandList() abstract;

	virtual uint32 GetSwapChainCurrentIndex() abstract;
	virtual IRHIResourceRef GetSwapChainResource( const uint32 index ) abstract;

	virtual IRHIPipelineRef CreatePipelineState( const RHIPipelineStateDesc& pipelineDesc ) abstract;

	virtual IRHIResourceRef CreateResource( const RHIResourceDesc& desc ) abstract;
	virtual IRHIResourceRef CreateResource( const RHIResourceDesc& desc, const void* data, uint32 sizeInBytes ) abstract;
	virtual IRHIRayTraceBottomLevelASRef CreateRayTraceBottomLevelAS( const RHIRayTraceBottomLevelASDesc& asDesc, const IRHIVertexBufferView& inVBV, const IRHIIndexBufferView& inIBV ) abstract;
	virtual IRHIRayTraceTopLevelASRef CreateRayTraceTopLevelAS( const std::vector<IRHIRayTraceBottomLevelASRef>& inBottomLevelAS, const RHIRayTraceTopLevelASDesc& asDesc ) abstract;

	virtual IRHIRenderTargetViewRef CreateRenderTargetView( const IRHIResource& inResource, const RHIRenderTargetViewDesc& rtvDesc ) abstract;
	virtual IRHIDepthStencilViewRef CreateDepthStencilView( const IRHIResource& inResource, const RHIDepthStencilViewDesc& dsvDesc ) abstract;
	virtual IRHIConstantBufferViewRef CreateConstantBufferView( const IRHIResource& inResource, const RHIConstantBufferViewDesc& cbvDesc ) abstract;
	virtual IRHIShaderResourceViewRef CreateShaderResourceView( const IRHIResource& inResource, const RHIShaderResourceViewDesc& srvDesc ) abstract;
	virtual IRHIUnorderedAccessViewRef CreateUnorderedAccessView( const IRHIResource& inResource, const RHIUnorderedAccessViewDesc& uavDesc, const bool bShaderHidden ) abstract;
	virtual IRHITextureViewTableRef CreateTextureViewTable( const std::vector<const IRHIResource*> inResources, const std::vector<RHIShaderResourceViewDesc> inDescs ) abstract;
	virtual IRHIDescriptorTableViewRef CreateDescriptorTableView( const std::vector<const IRHIResource*>& inResources,
																  const std::vector<RHIConstantBufferViewDesc>& inCBVDescs,
																  const std::vector<RHIShaderResourceViewDesc>& inSRVDescs,
																  const std::vector<RHIUnorderedAccessViewDesc>& inUAVDescs ) abstract;
	virtual IRHISamplerRef CreateSampler( const IRHIResource& inResource, const RHISamplerDesc& samplerDesc ) abstract;
	virtual IRHIVertexBufferViewRef CreateVertexBufferView( const IRHIResource& inResource, const uint32 size, const uint32 stride ) abstract;
	virtual IRHIIndexBufferViewRef CreateIndexBufferView( const IRHIResource& inResource, const uint32 size ) abstract;

	virtual void CopyBufferRegion( const IRHIResource& inCopyDst, const IRHIResource& inCopySrc, const uint32 copyDstByteOffset, const uint32 copyByteSize ) abstract;

	virtual void BindRenderTargetView( IRHIPipeline& inPipeline, const IRHIRenderTargetView& inRTV, uint32 index ) abstract;
	virtual void BindConstantBufferView( IRHIPipeline& inPipeline, const IRHIConstantBufferView& inCBV, uint32 index ) abstract;
	virtual void BindShaderResourceView( IRHIPipeline& inPipeline, const IRHIShaderResourceView& inSRV, uint32 index ) abstract;
	virtual void BindUnorderedAccessView( IRHIPipeline& inPipeline, const IRHIUnorderedAccessView& inUAV, uint32 index ) abstract;
	virtual void BindDepthStencilView( IRHIPipeline& inPipeline, const IRHIDepthStencilView& inDSV ) abstract;
	virtual void BindTextureViewTable( IRHIPipeline& inPipeline, const IRHITextureViewTable& inTable, const uint32 index ) abstract;

	virtual void UpdateResourceData( const IRHIResource& inResource, void* data, uint32 sizeInBytes ) abstract;
	virtual void UpdateTextureData( const IRHIResource& inResource, void* data, uint32 width, uint32 height ) abstract;

	virtual void TransitionResource( const IRHIResource& inResource, const ERHIResourceStates statesBefore, const ERHIResourceStates statesAfter ) abstract;

	virtual void RunCS() abstract;

	virtual void RayTrace( const RHIPipelineStateDesc& desc, const IRHIRayTraceTopLevelASRef& inRTRAS, IRHIDescriptorTableViewRef descTableView, IRHIShaderResourceViewRef testNormalSRV, IRHIShaderResourceViewRef testIndexSRV, IRHIShaderResourceViewRef testIndexOffsetSRV, IRHIShaderResourceViewRef testMaterialSRV ) abstract;

	void SetWindowSize( const IVec2& size ) { _windowSize = size; }
	IVec2 GetWindowSize() { return _windowSize; }

protected:
	IVec2 _windowSize;
};