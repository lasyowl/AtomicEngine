#pragma once

#include "EngineEssential.h"
#include "GPI.h"
#include "GPIDefine.h"
#include "GPIResource_DX12.h"

struct IDXGISwapChain;
struct GPIPipeline_DX12;
struct RawImage;
struct ConstantBuffer;

constexpr int32 CMD_LIST_PER_QUEUE_COUNT = 3;
constexpr int32 SWAPCHAIN_COUNT = 3;

struct CommandQueueContext
{
	ID3D12CommandQueue* cmdQueue;
	ID3D12CommandAllocator* allocator;

	std::array<ID3D12GraphicsCommandList*, CMD_LIST_PER_QUEUE_COUNT> cmdLists;
	std::array<ID3D12GraphicsCommandList*, CMD_LIST_PER_QUEUE_COUNT>::iterator cmdListIter;

	ID3D12Fence* fence;
	HANDLE fenceEventHandle;
	uint64 fenceValue;
};

/* @TODO : remove? */
class GPIResourceAllocator_DX12
{
public:
	void Initialize();

	uint32 AllocateUniqueResourceID( ID3D12Resource* resource );
	void ReleaseUniqueResourceID( uint32 resourceID );

	uint32 CreateResource( ID3D12Device* device, const GPIResourceDesc& desc );
	ID3D12Resource* GetResource( uint32 resourceID );

private:
	std::queue<uint32> _freeResourceIDs;
	std::unordered_map<uint32, ID3D12Resource*> _resourceCache;
};

struct GPIDescriptorHeap_DX12
{
	uint64 descSize;
	ID3D12DescriptorHeap* heap;

	std::queue<GPIDescriptorHeapHandle_DX12> free;
	std::unordered_set<GPIDescriptorHeapHandle_DX12> used;
};

class GPIDescriptorHeapAllocator_DX12
{
public:
	void Initialize( ID3D12Device* device );

	GPIDescriptorHeapHandle_DX12 Allocate( const EGPIResourceViewType type );
	void Release( const EGPIResourceViewType type, const GPIDescriptorHeapHandle_DX12& handle );

private:
	std::array<GPIDescriptorHeap_DX12, GPIResourceViewTypeSize> _heapContexts;
};

class GPI_DX12 : public IGPI
{
public:
	GPI_DX12( const HWND hWnd, const int32 screenWidth, const int32 screenHeight );

	virtual void Initialize() override;
	virtual void BeginFrame( const IGPIResource& inSwapChainResource, const IGPIRenderTargetView& inSwapChainRTV, const IGPIDepthStencilView& inSwapChainDSV ) override;
	virtual void EndFrame( const IGPIResource& inSwapChainResource ) override;

	virtual void ClearSwapChain( IGPIRenderTargetView* inRTV ) override;
	virtual void ClearRenderTarget( IGPIRenderTargetView* inRTV ) override;

	virtual void SetPipelineState( const GPIPipelineStateDesc& pipelineDesc ) override;
	virtual void Render( const GPIPipelineInput& pipelineInput ) override;
	virtual void ExecuteCommandList() override;

	virtual uint32 GetSwapChainCurrentIndex() override;
	virtual IGPIResourceRef GetSwapChainResource( const uint32 index ) override;

	virtual IGPIPipelineRef CreatePipelineState( const GPIPipelineStateDesc& pipelineDesc ) override;

	virtual IGPIResourceRef CreateResource( const GPIResourceDesc& desc ) override;
	virtual IGPIResourceRef CreateResource( const GPIResourceDesc& desc, void* data, uint32 sizeInBytes ) override;

	virtual IGPIRenderTargetViewRef CreateRenderTargetView( const IGPIResource& inResource, const GPIRenderTargetViewDesc& rtvDesc ) override;
	virtual IGPIDepthStencilViewRef CreateDepthStencilView( const IGPIResource& inResource, const GPIDepthStencilViewDesc& dsvDesc ) override;
	virtual IGPIConstantBufferViewRef CreateConstantBufferView( const IGPIResource& inResource, const GPIConstantBufferViewDesc& cbvDesc ) override;
	virtual IGPIShaderResourceViewRef CreateShaderResourceView( const IGPIResource& inResource, const GPIShaderResourceViewDesc& srvDesc ) override;
	virtual IGPIUnorderedAccessViewRef CreateUnorderedAccessView( const IGPIResource& inResource, const GPIUnorderedAccessViewDesc& uavDesc ) override;
	virtual IGPISamplerRef CreateSampler( const IGPIResource& inResource, const GPISamplerDesc& samplerDesc ) override;
	virtual IGPIVertexBufferViewRef CreateVertexBufferView( const IGPIResource& inResource, const uint32 size, const uint32 stride ) override;
	virtual IGPIIndexBufferViewRef CreateIndexBufferView( const IGPIResource& inResource, const uint32 size ) override;

	virtual void BindRenderTargetView( IGPIPipeline& inPipeline, const IGPIRenderTargetView& inRTV, uint32 index ) override;
	virtual void BindConstantBufferView( IGPIPipeline& inPipeline, const IGPIConstantBufferView& inCBV, uint32 index ) override;
	virtual void BindShaderResourceView( IGPIPipeline& inPipeline, const IGPIShaderResourceView& inSRV, uint32 index ) override;
	virtual void BindUnorderedAccessView( IGPIPipeline& inPipeline, const IGPIUnorderedAccessView& inUAV, uint32 index ) override;
	virtual void BindDepthStencilView( IGPIPipeline& inPipeline, const IGPIDepthStencilView& inDSV ) override;

	virtual void UpdateResourceData( const IGPIResource& inResource, void* data, uint32 sizeInBytes ) override;

	//virtual void UpdateConstantBuffer( uint32 bufferID, void* data, uint32 size ) override;

	virtual void RunCS() override;

private:
	void SetPipelineState( const GPIPipelineStateDesc& desc, const GPIPipeline_DX12& pipeline );

private:
	HWND _hWnd;

	GPIResourceAllocator_DX12 _resource;
	GPIDescriptorHeapAllocator_DX12 _heap;

	ID3D12Device* _device;
	IDXGISwapChain* _swapChain;

	ID3D12Debug* _debugInterface;
	ID3D12InfoQueue* _debugInfoQueue;

	std::unordered_map<D3D12_COMMAND_LIST_TYPE, CommandQueueContext> _cmdQueueCtx;

	int32 _swapChainIndex;

	std::unordered_map<uint32, std::shared_ptr<GPIPipeline_DX12>> _pipelineCache;
	//std::unordered_map<uint32, ID3DBlob*> _shaderCache;

	//std::unordered_map<uint32, ID3D12Resource*> _resourceCache;

	//std::shared_ptr<RawImage> rawImage;
	//ID3D12Resource* textureBuffer;
};
