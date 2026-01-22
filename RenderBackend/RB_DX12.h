#pragma once

#include <Core/IntVector.h>
#include <RenderBackend/RB.h>
#include <RenderBackend/RBDefine.h>
#include <RenderBackend/RBResource_DX12.h>

struct IDXGISwapChain;
struct RBPipeline_DX12;

constexpr int32 kCmdListPerQueueCount = 3;
constexpr int32 kSwapChainCount = 3;

struct CommandQueueContext
{
    ID3D12CommandQueue* cmdQueue;
    ID3D12CommandAllocator* allocator;

    std::array<ID3D12GraphicsCommandList*, kCmdListPerQueueCount> cmdLists;
    std::array<ID3D12GraphicsCommandList*, kCmdListPerQueueCount>::iterator iCmdList;

    ID3D12Fence* fence;
    HANDLE fenceEventHandle;
    uint64 fenceValue;
};

/* @TODO : remove? */
class RBResourceAllocator_DX12
{
public:
    void Initialize();

    uint32 AllocateUniqueResourceID( ID3D12Resource* resource );
    void ReleaseUniqueResourceID( uint32 resourceID );

    uint32 CreateResource( ID3D12Device* device, const RBResourceDesc& desc );
    ID3D12Resource* GetResource( uint32 resourceID );

private:
    std::queue<uint32> _freeResourceIDs;
    std::unordered_map<uint32, ID3D12Resource*> _resourceCache;
};

struct RBDescriptorHeap_DX12
{
    uint64 descSize;
    ID3D12DescriptorHeap* heap;

    D3D12_CPU_DESCRIPTOR_HANDLE hCPUStaticCurrent;
    D3D12_GPU_DESCRIPTOR_HANDLE hGPUStaticCurrent;

    D3D12_CPU_DESCRIPTOR_HANDLE hCPUDynamicBegin;
    D3D12_CPU_DESCRIPTOR_HANDLE hCPUDynamicCurrent;
    D3D12_GPU_DESCRIPTOR_HANDLE hGPUDynamicBegin;
    D3D12_GPU_DESCRIPTOR_HANDLE hGPUDynamicCurrent;
};

class RBDescriptorHeapAllocator_DX12
{
public:
    void Initialize( ID3D12Device* device );
    void Release();

    RBDescriptorHandle_DX12 AllocateStatic( const ERBResourceViewType type );
    RBDescriptorHandle_DX12 AllocateStaticConsecutive( const ERBResourceViewType type, const uint32 count );
    RBDescriptorHandle_DX12 AllocateDynamic( const ERBResourceViewType type );
    RBDescriptorHandle_DX12 AllocateDynamicConsecutive( const ERBResourceViewType type, const uint32 count );
    void ClearDynamic();

    ID3D12DescriptorHeap* GetHeap( const ERBResourceViewType type );

private:
    std::array<RBDescriptorHeap_DX12, RBResourceViewTypeSize> _heap;
};

class RB_DX12 : public IRenderBackend
{
public:
    RB_DX12( const HWND hWnd, const IVec2& windowSize );
    virtual ~RB_DX12();

    virtual void Startup() override;
    virtual void BeginFrame() override;
    virtual void EndFrame() override;

    virtual void ClearRenderTarget( const IRBResource& inResource, const IRBRenderTargetView& inRTV ) override;

    virtual void SetGraphicsPipelineState( const IRBPipeline& inPipeline ) override;

    virtual void SetInputAssembly( const IRBVertexBufferView& inVbv ) override;
    virtual void SetInputAssembly( const IRBVertexBufferView& inVbv, const IRBIndexBufferView& inIbv ) override;
    virtual void DrawInstanced( uint32 numVertices, uint32 numInstances, uint32 vertexOffset, uint32 instanceOffset ) override;
    virtual void DrawIndexedInstanced( uint32 numIndices, uint32 numInstances, uint32 indexOffset, uint32 vertexOffset, uint32 instanceOffset ) override;
    virtual void ExecuteCommandList() override;

    virtual IRBResource* GetSwapChainColorResource() override;
    virtual IRBResource* GetSwapChainDepthResource() override;
    virtual IRBRenderTargetView* GetSwapChainRenderTargetView() override;
    virtual IRBDepthStencilView* GetSwapChainDepthStencilView() override;

    virtual IRBShaderPtr CreateShader( const RBShaderDesc& shaderDesc ) override;

    virtual IRBPipelineRef CreateGraphicsPipelineState( const RBPipelineStateDesc& pipelineDesc ) override;

    virtual IRBResourcePtr CreateResource( const RBResourceDesc& desc ) override;
    virtual IRBResourcePtr CreateResource( const RBResourceDesc& desc, const void* data, uint32 sizeInBytes ) override;
    virtual IRBRayTraceBottomLevelASRef CreateRayTraceBottomLevelAS( const RBRayTraceBottomLevelASDesc& asDesc, const IRBVertexBufferView& inVBV, const IRBIndexBufferView& inIBV ) override;
    virtual IRBRayTraceTopLevelASRef CreateRayTraceTopLevelAS( const std::vector<IRBRayTraceBottomLevelASRef>& inBottomLevelAS, const RBRayTraceTopLevelASDesc& asDesc ) override;

    virtual IRBRenderTargetViewPtr CreateRenderTargetView( const IRBResource& inResource, const RBRenderTargetViewDesc& rtvDesc ) override;
    virtual IRBDepthStencilViewRef CreateDepthStencilView( const IRBResource& inResource, const RBDepthStencilViewDesc& dsvDesc ) override;
    virtual IRBConstantBufferViewPtr CreateConstantBufferView( const IRBResource& inResource, const RBConstantBufferViewDesc& cbvDesc ) override;
    virtual IRBShaderResourceViewPtr CreateShaderResourceView( const IRBResource& inResource, const RBShaderResourceViewDesc& srvDesc ) override;
    virtual IRBUnorderedAccessViewPtr CreateUnorderedAccessView( const IRBResource& inResource, const RBUnorderedAccessViewDesc& uavDesc, const bool bShadeRBdden ) override;
    virtual IRBTextureViewTablePtr CreateTextureViewTable( uint32 resourceCount ) override;
    virtual IRBDescriptorTableViewPtr CreateDescriptorTableView( const std::vector<const IRBResource*>& inResources,
                                                                  const std::vector<RBConstantBufferViewDesc>& inCBVDescs,
                                                                  const std::vector<RBShaderResourceViewDesc>& inSRVDescs,
                                                                  const std::vector<RBUnorderedAccessViewDesc>& inUAVDescs ) override;
    virtual IRBSamplerPtr CreateSampler( const IRBResource& inResource, const RBSamplerDesc& samplerDesc ) override;
    virtual IRBVertexBufferViewPtr CreateVertexBufferView( const IRBResource& inResource, const uint32 size, const uint32 stride ) override;
    virtual IRBIndexBufferViewPtr CreateIndexBufferView( const IRBResource& inResource, const uint32 size ) override;

    virtual void CopyBufferRegion( const IRBResource& inCopyDst, const IRBResource& inCopySrc, const uint32 copyDstByteOffset, const uint32 copyByteSize ) override;
    virtual void CopyTextureRegion( const IRBResource& inCopyDst, const IRBResource& inCopySrc, uint32 width, uint32 height ) override;

    virtual void BindConstantBufferView( const IRBConstantBufferView& inCBV, uint32 index ) override;
    virtual void BindShaderResourceView( const IRBShaderResourceView& inSRV, uint32 index ) override;
    virtual void BindUnorderedAccessView( const IRBUnorderedAccessView& inUAV, uint32 index ) override;
    virtual void BindTextureViewTable( const IRBTextureViewTable& inTable, const uint32 index ) override;
    virtual void BindRenderTargetViews( std::span<const IRBRenderTargetView*> inRTVs, const IRBDepthStencilView* inDSV ) override;

    virtual void UpdateResourceData( const IRBResource& inResource, const void* data, uint32 sizeInBytes ) override;
    virtual void UpdateTextureData( const IRBResource& inResource, const void* data, uint32 width, uint32 height ) override;
    virtual void UpdateTextureViewTable( const IRBTextureViewTable& inTable, const IRBShaderResourceView& inSRV, uint32 index ) override;

    virtual void TransitionResource( const IRBResource& inResource, const ERBResourceStates statesBefore, const ERBResourceStates statesAfter ) override;

    virtual void RunCS() override;

private:
    void SetComputePipelineState( const RBPipeline_DX12& pipeline );
    void SetRayTracePipelineState( const RBPipeline_DX12& pipeline );

    ID3D12Resource* CreateResource_Inner( const RBResourceDesc& desc, const void* data, uint32 sizeInBytes );

    void TransitionResource( ID3D12GraphicsCommandList* cmdList, ID3D12Resource* resource, D3D12_RESOURCE_STATES stateBefore, D3D12_RESOURCE_STATES stateAfter );

private:
    HWND _hWnd;

    RBResourceAllocator_DX12 _resource;
    RBDescriptorHeapAllocator_DX12 _heap;

    ID3D12Device* _device;
    IDXGISwapChain* _swapChain;

    ID3D12Debug* _debugInterface;

    ID3D12DeviceRemovedExtendedDataSettings* _debugInterfaceEx;

    std::unordered_map<D3D12_COMMAND_LIST_TYPE, CommandQueueContext> _cmdQueueCtx;

    int32 _swapChainIndex;
    RBSwapChain_DX12 _swapChainResources[ kSwapChainCount ];
};
