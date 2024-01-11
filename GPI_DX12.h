#pragma once

#include "EngineEssential.h"
#include "GPI.h"
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

class GPI_DX12 : public IGPI
{
public:
	GPI_DX12( const HWND hWnd, const int32 screenWidth, const int32 screenHeight );

	virtual void Initialize() override;
	virtual void BeginFrame() override;
	virtual void EndFrame() override;

	virtual void SetPipelineState( const GPIPipelineStateDesc& pipelineDesc ) override;
	virtual void Render( IVertexBuffer* positionBuffer, IVertexBuffer* uvBuffer, IVertexBuffer* normalBuffer, IIndexBuffer* indexBuffer ) override;
	virtual void FlushPipelineState() override;

	virtual IVertexBufferRef CreateVertexBuffer( void* data, uint32 stride, uint32 size ) override;
	virtual IIndexBufferRef CreateIndexBuffer( void* data, uint32 size ) override;

	virtual void CreatePipelineState( const GPIPipelineStateDesc& pipelineStateDesc ) override;

	// todo : integrate with UpdateConstBuffer1
	virtual void UpdateConstantBuffer( uint32 bufferHash, void* data, uint32 size ) override;
	virtual void UpdateConstantBuffer1( const GPIPipelineStateDesc& pipelineDesc, void* data ) override;

	virtual void RunCS() override;

private:
	ID3D12Resource* CreateConstantBuffer( void* data, uint32 size );

	void SetPipelineState( const GPIPipelineStateDesc& desc, ID3D12PipelineState* pso, ID3D12RootSignature* rootSignature, ID3D12Resource* constBuffer );

private:
	HWND _hWnd;
	int32 _screenWidth;
	int32 _screenHeight;

	ID3D12Device* _device;
	IDXGISwapChain* _swapChain;

	ID3D12DescriptorHeap* _rtvHeap;
	ID3D12DescriptorHeap* _dsvHeap;

	/* Heap includes static per frame resource view descriptors( constant buffer, shader resource, unordered access ). */
	ID3D12DescriptorHeap* _rvHeap;

	// todo : separate for compute shader
	ID3D12DescriptorHeap* _uavHeap;
	ID3D12DescriptorHeap* _guavHeap;
	ID3D12DescriptorHeap* _srvHeap;
	ID3D12DescriptorHeap* _cbvHeap;

	ID3D12Debug* _debugInterface;
	ID3D12InfoQueue* _debugInfoQueue;

	std::unordered_map<D3D12_COMMAND_LIST_TYPE, CommandQueueContext> _cmdQueueCtx;

	std::array<ID3D12Resource*, SWAPCHAIN_COUNT> _swapChainBuffers;

	int32 _swapChainIndex;

	std::unordered_map<uint32, std::shared_ptr<GPIPipeline_DX12>> _pipelineCache;
	std::unordered_map<uint32, ID3DBlob*> _shaderCache;
	std::unordered_map<uint32, ID3D12Resource*> _constBufferCache;

	ID3D12Resource* _constBuffer; // temp
	ID3D12Resource* _gBuffers[ 4 ];

	ID3D12Resource* _depthStencilBuffer;

	std::shared_ptr<RawImage> rawImage;
	ID3D12Resource* textureBuffer;
};
