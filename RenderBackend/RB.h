#pragma once

#include <Core/IntVector.h>
#include <RenderBackend/RBShader.h>
#include <RenderBackend/RBPipeline.h>
#include <RenderBackend/RBResource.h>

struct IRBPipeline;
struct RBRenderTargetViewDesc;
struct RBDepthStencilViewDesc;
struct RBConstantBufferViewDesc;
struct RBShaderResourceViewDesc;
struct RBUnorderedAccessViewDesc;
struct RBSamplerDesc;
struct RBShaderDesc;
struct RBPipelineStateDesc;

/* Render Backend Interface */
class IRenderBackend
{
public:
    IRenderBackend() = default;
    virtual ~IRenderBackend() = default;

    virtual void Startup() abstract;
    virtual void BeginFrame() abstract;
    virtual void EndFrame() abstract;

    virtual void ClearRenderTarget( const IRBResource& inResource, const IRBRenderTargetView& inRTV ) abstract;

    virtual void SetGraphicsPipelineState( const IRBPipeline& inPipeline ) abstract;

    virtual void SetInputAssembly( const IRBVertexBufferView& inVbv ) abstract;
    virtual void SetInputAssembly( const IRBVertexBufferView& inVbv, const IRBIndexBufferView& inIbv ) abstract;
    virtual void DrawInstanced( uint32 numVertices, uint32 numInstances, uint32 vertexOffset, uint32 instanceOffset ) abstract;
    virtual void DrawIndexedInstanced( uint32 numIndices, uint32 numInstances, uint32 indexOffset, uint32 vertexOffset, uint32 instanceOffset ) abstract;
    virtual void ExecuteCommandList() abstract;

    virtual IRBResource* GetSwapChainColorResource() abstract;
    virtual IRBResource* GetSwapChainDepthResource() abstract;
    virtual IRBRenderTargetView* GetSwapChainRenderTargetView() abstract;
    virtual IRBDepthStencilView* GetSwapChainDepthStencilView() abstract;

    virtual IRBShaderPtr CreateShader( const RBShaderDesc& shaderDesc ) abstract;
        
    virtual IRBPipelineRef CreateGraphicsPipelineState( const RBPipelineStateDesc& pipelineDesc ) abstract;

    virtual IRBResourcePtr CreateResource( const RBResourceDesc& desc ) abstract;
    virtual IRBResourcePtr CreateResource( const RBResourceDesc& desc, const void* data, uint32 sizeInBytes ) abstract;
    virtual IRBRayTraceBottomLevelASRef CreateRayTraceBottomLevelAS( const RBRayTraceBottomLevelASDesc& asDesc, const IRBVertexBufferView& inVBV, const IRBIndexBufferView& inIBV ) abstract;
    virtual IRBRayTraceTopLevelASRef CreateRayTraceTopLevelAS( const std::vector<IRBRayTraceBottomLevelASRef>& inBottomLevelAS, const RBRayTraceTopLevelASDesc& asDesc ) abstract;

    virtual IRBRenderTargetViewPtr CreateRenderTargetView( const IRBResource& inResource, const RBRenderTargetViewDesc& rtvDesc ) abstract;
    virtual IRBDepthStencilViewRef CreateDepthStencilView( const IRBResource& inResource, const RBDepthStencilViewDesc& dsvDesc ) abstract;
    virtual IRBConstantBufferViewPtr CreateConstantBufferView( const IRBResource& inResource, const RBConstantBufferViewDesc& cbvDesc ) abstract;
    virtual IRBShaderResourceViewPtr CreateShaderResourceView( const IRBResource& inResource, const RBShaderResourceViewDesc& srvDesc ) abstract;
    virtual IRBUnorderedAccessViewPtr CreateUnorderedAccessView( const IRBResource& inResource, const RBUnorderedAccessViewDesc& uavDesc, const bool bShadeRBdden ) abstract;
    virtual IRBTextureViewTablePtr CreateTextureViewTable( uint32 resourceCount ) abstract;
    virtual IRBDescriptorTableViewPtr CreateDescriptorTableView( const std::vector<const IRBResource*>& inResources,
                                                                  const std::vector<RBConstantBufferViewDesc>& inCBVDescs,
                                                                  const std::vector<RBShaderResourceViewDesc>& inSRVDescs,
                                                                  const std::vector<RBUnorderedAccessViewDesc>& inUAVDescs ) abstract;
    virtual IRBSamplerPtr CreateSampler( const IRBResource& inResource, const RBSamplerDesc& samplerDesc ) abstract;
    virtual IRBVertexBufferViewPtr CreateVertexBufferView( const IRBResource& inResource, const uint32 size, const uint32 stride ) abstract;
    virtual IRBIndexBufferViewPtr CreateIndexBufferView( const IRBResource& inResource, const uint32 size ) abstract;

    virtual void CopyBufferRegion( const IRBResource& inCopyDst, const IRBResource& inCopySrc, const uint32 copyDstByteOffset, const uint32 copyByteSize ) abstract;
    virtual void CopyTextureRegion( const IRBResource& inCopyDst, const IRBResource& inCopySrc, uint32 width, uint32 height ) abstract;

    virtual void BindConstantBufferView( const IRBConstantBufferView& inCBV, uint32 index ) abstract;
    virtual void BindShaderResourceView( const IRBShaderResourceView& inSRV, uint32 index ) abstract;
    virtual void BindUnorderedAccessView( const IRBUnorderedAccessView& inUAV, uint32 index ) abstract;
    virtual void BindTextureViewTable( const IRBTextureViewTable& inTable, const uint32 index ) abstract;
    virtual void BindRenderTargetViews( std::span<const IRBRenderTargetView*> inRTVs, const IRBDepthStencilView* inDSV ) abstract;

    virtual void UpdateResourceData( const IRBResource& inResource, const void* data, uint32 sizeInBytes ) abstract;
    virtual void UpdateTextureData( const IRBResource& inResource, const void* data, uint32 width, uint32 height ) abstract;
    virtual void UpdateTextureViewTable( const IRBTextureViewTable& inTable, const IRBShaderResourceView& inSRV, uint32 index ) abstract;

    virtual void TransitionResource( const IRBResource& inResource, const ERBResourceStates statesBefore, const ERBResourceStates statesAfter ) abstract;

    virtual void RunCS() abstract;

    inline void SetWindowSize( const IVec2& size ) { _windowSize = size; }
    inline IVec2 GetWindowSize() const { return _windowSize; }

private:
    IVec2 _windowSize;
};