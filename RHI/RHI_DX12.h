#pragma once

#include <Core/IntVector.h>
#include <RHI/RHI.h>
#include <RHI/RHIDefine.h>
#include <RHI/RHIResource_DX12.h>

struct IDXGISwapChain;
struct RHIPipeline_DX12;
struct ConstantBuffer;

constexpr int32 CMD_LIST_PER_QUEUE_COUNT = 3;
constexpr int32 SWAPCHAIN_COUNT = 3;

struct CommandQueueContext
{
	ID3D12CommandQueue* cmdQueue;
	ID3D12CommandAllocator* allocator;

	std::array<ID3D12GraphicsCommandList*, CMD_LIST_PER_QUEUE_COUNT> cmdLists;
	std::array<ID3D12GraphicsCommandList*, CMD_LIST_PER_QUEUE_COUNT>::iterator iCmdList;

	ID3D12Fence* fence;
	HANDLE fenceEventHandle;
	uint64 fenceValue;
};

/* @TODO : remove? */
class RHIResourceAllocator_DX12
{
public:
	void Initialize();

	uint32 AllocateUniqueResourceID( ID3D12Resource* resource );
	void ReleaseUniqueResourceID( uint32 resourceID );

	uint32 CreateResource( ID3D12Device* device, const RHIResourceDesc& desc );
	ID3D12Resource* GetResource( uint32 resourceID );

private:
	std::queue<uint32> _freeResourceIDs;
	std::unordered_map<uint32, ID3D12Resource*> _resourceCache;
};

struct RHIDescriptorHeap_DX12
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

class RHIDescriptorHeapAllocator_DX12
{
public:
	void Initialize( ID3D12Device* device );
	void Release();

	RHIDescriptorHeapHandle_DX12 AllocateStatic( const ERHIResourceViewType type );
	RHIDescriptorHeapHandle_DX12 AllocateStaticConsecutive( const ERHIResourceViewType type, const uint32 count );
	RHIDescriptorHeapHandle_DX12 AllocateDynamic( const ERHIResourceViewType type );
	RHIDescriptorHeapHandle_DX12 AllocateDynamicConsecutive( const ERHIResourceViewType type, const uint32 count );
	void ClearDynamic();

	ID3D12DescriptorHeap* GetHeap( const ERHIResourceViewType type );

private:
	std::array<RHIDescriptorHeap_DX12, RHIResourceViewTypeSize> _heap;
};

class RHI_DX12 : public IRHI
{
public:
	RHI_DX12( const HWND hWnd, const IVec2& windowSize );
	virtual ~RHI_DX12();

	virtual void Initialize() override;
	virtual void BeginFrame( const IRHIResource& inSwapChainResource, const IRHIRenderTargetView& inSwapChainRTV, const IRHIDepthStencilView& inSwapChainDSV ) override;
	virtual void EndFrame( const IRHIResource& inSwapChainResource ) override;

	virtual void ClearSwapChain( const IRHIRenderTargetView& inRTV ) override;
	virtual void ClearRenderTarget( const IRHIRenderTargetView& inRTV ) override;
	virtual void ClearRenderTarget( const IRHIUnorderedAccessView& inUAV ) override;

	virtual void SetPipelineState( const RHIPipelineStateDesc& pipelineDesc ) override;
	virtual void Render( const RHIPipelineInput& pipelineInput ) override;
	virtual void ExecuteCommandList() override;

	virtual uint32 GetSwapChainCurrentIndex() override;
	virtual IRHIResourceRef GetSwapChainResource( const uint32 index ) override;

	virtual IRHIPipelineRef CreatePipelineState( const RHIPipelineStateDesc& pipelineDesc ) override;

	virtual IRHIResourceRef CreateResource( const RHIResourceDesc& desc ) override;
	virtual IRHIResourceRef CreateResource( const RHIResourceDesc& desc, const void* data, uint32 sizeInBytes ) override;
	virtual IRHIRayTraceBottomLevelASRef CreateRayTraceBottomLevelAS( const RHIRayTraceBottomLevelASDesc& asDesc, const IRHIVertexBufferView& inVBV, const IRHIIndexBufferView& inIBV ) override;
	virtual IRHIRayTraceTopLevelASRef CreateRayTraceTopLevelAS( const std::vector<IRHIRayTraceBottomLevelASRef>& inBottomLevelAS, const RHIRayTraceTopLevelASDesc& asDesc ) override;

	virtual IRHIRenderTargetViewRef CreateRenderTargetView( const IRHIResource& inResource, const RHIRenderTargetViewDesc& rtvDesc ) override;
	virtual IRHIDepthStencilViewRef CreateDepthStencilView( const IRHIResource& inResource, const RHIDepthStencilViewDesc& dsvDesc ) override;
	virtual IRHIConstantBufferViewRef CreateConstantBufferView( const IRHIResource& inResource, const RHIConstantBufferViewDesc& cbvDesc ) override;
	virtual IRHIShaderResourceViewRef CreateShaderResourceView( const IRHIResource& inResource, const RHIShaderResourceViewDesc& srvDesc ) override;
	virtual IRHIUnorderedAccessViewRef CreateUnorderedAccessView( const IRHIResource& inResource, const RHIUnorderedAccessViewDesc& uavDesc, const bool bShaderHidden ) override;
	virtual IRHITextureViewTableRef CreateTextureViewTable( const std::vector<const IRHIResource*> inResources, const std::vector<RHIShaderResourceViewDesc> inDescs ) override;
	virtual IRHIDescriptorTableViewRef CreateDescriptorTableView( const std::vector<const IRHIResource*>& inResources,
																  const std::vector<RHIConstantBufferViewDesc>& inCBVDescs,
																  const std::vector<RHIShaderResourceViewDesc>& inSRVDescs,
																  const std::vector<RHIUnorderedAccessViewDesc>& inUAVDescs ) override;
	virtual IRHISamplerRef CreateSampler( const IRHIResource& inResource, const RHISamplerDesc& samplerDesc ) override;
	virtual IRHIVertexBufferViewRef CreateVertexBufferView( const IRHIResource& inResource, const uint32 size, const uint32 stride ) override;
	virtual IRHIIndexBufferViewRef CreateIndexBufferView( const IRHIResource& inResource, const uint32 size ) override;

	virtual void CopyBufferRegion( const IRHIResource& inCopyDst, const IRHIResource& inCopySrc, const uint32 copyDstByteOffset, const uint32 copyByteSize ) override;

	virtual void BindRenderTargetView( IRHIPipeline& inPipeline, const IRHIRenderTargetView& inRTV, uint32 index ) override;
	virtual void BindConstantBufferView( IRHIPipeline& inPipeline, const IRHIConstantBufferView& inCBV, uint32 index ) override;
	virtual void BindShaderResourceView( IRHIPipeline& inPipeline, const IRHIShaderResourceView& inSRV, uint32 index ) override;
	virtual void BindUnorderedAccessView( IRHIPipeline& inPipeline, const IRHIUnorderedAccessView& inUAV, uint32 index ) override;
	virtual void BindDepthStencilView( IRHIPipeline& inPipeline, const IRHIDepthStencilView& inDSV ) override;
	virtual void BindTextureViewTable( IRHIPipeline& inPipeline, const IRHITextureViewTable& inTable, const uint32 index ) override;

	virtual void UpdateResourceData( const IRHIResource& inResource, void* data, uint32 sizeInBytes ) override;
	virtual void UpdateTextureData( const IRHIResource& inResource, void* data, uint32 width, uint32 height ) override;

	virtual void TransitionResource( const IRHIResource& inResource, const ERHIResourceStates statesBefore, const ERHIResourceStates statesAfter ) override;

	virtual void RunCS() override;

	virtual void RayTrace( const RHIPipelineStateDesc& desc, const IRHIRayTraceTopLevelASRef& inRTRAS, IRHIDescriptorTableViewRef descTableView, IRHIShaderResourceViewRef testNormalSRV, IRHIShaderResourceViewRef testIndexSRV, IRHIShaderResourceViewRef testIndexOffsetSRV, IRHIShaderResourceViewRef testMaterialSRV ) override;

private:
	void SetGraphicsPipelineState( const RHIPipelineStateDesc& desc, const RHIPipeline_DX12& pipeline );
	void SetComputePipelineState( const RHIPipelineStateDesc& desc, const RHIPipeline_DX12& pipeline );
	void SetRayTracePipelineState( const RHIPipelineStateDesc& desc, const RHIPipeline_DX12& pipeline );

	ID3D12Resource* CreateResource_Inner( const RHIResourceDesc& desc, const void* data, uint32 sizeInBytes );

	void TransitionResource( ID3D12GraphicsCommandList* cmdList, ID3D12Resource* resource, D3D12_RESOURCE_STATES stateBefore, D3D12_RESOURCE_STATES stateAfter );

private:
	HWND _hWnd;

	RHIResourceAllocator_DX12 _resource;
	RHIDescriptorHeapAllocator_DX12 _heap;

	ID3D12Device* _device;
	IDXGISwapChain* _swapChain;

	ID3D12Debug* _debugInterface;

	ID3D12DeviceRemovedExtendedDataSettings* _debugInterfaceEx;

	std::unordered_map<D3D12_COMMAND_LIST_TYPE, CommandQueueContext> _cmdQueueCtx;

	int32 _swapChainIndex;

	std::unordered_map<uint32, std::shared_ptr<RHIPipeline_DX12>> _pipelineCache;
	//std::unordered_map<uint32, ID3DBlob*> _shaderCache;

	//std::unordered_map<uint32, ID3D12Resource*> _resourceCache;
};
