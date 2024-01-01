#pragma once

#include "EngineEssential.h"
#include "GPI.h"
#include "GPIResource_DX12.h"

struct ID3D12DescriptorHeap;
struct ID3D12PipelineState;
struct ID3D12RootSignature;
struct ID3D12GraphicsCommandList;

constexpr int32 CMD_LIST_PER_QUEUE_COUNT = 3;
constexpr int32 SWAPCHAIN_BUFFER_COUNT = 3;

struct CommandQueueContext
{
	struct ID3D12CommandQueue* cmdQueue;
	struct ID3D12CommandAllocator* allocator;

	std::array<ID3D12GraphicsCommandList*, CMD_LIST_PER_QUEUE_COUNT> cmdLists;
	std::array<ID3D12GraphicsCommandList*, CMD_LIST_PER_QUEUE_COUNT>::iterator cmdListIter;

	struct ID3D12Fence* fence;
	HANDLE fenceEventHandle;
	uint64 fenceValue;
};

struct SwapChainBufferContext
{
	ID3D12Resource* renderTarget;
	uint32 bufferIndex;
	size_t renderTargetHandlePtr;
};

class GPI_DX12 : public IGPI
{
public:
	GPI_DX12( const HWND hWnd, const int32 screenWidth, const int32 screenHeight );

	virtual void Initialize() override;
	virtual void BeginFrame() override;
	virtual void EndFrame() override;

	virtual void SetPipelineState( uint32 pipelineStateHash ) override;
	virtual void Render( IVertexBuffer* positionBuffer, IVertexBuffer* uvBuffer, IIndexBuffer* indexBuffer ) override;
	virtual void FlushPipelineState() override;

	virtual IVertexBufferRef CreateVertexBuffer( void* data, uint32 stride, uint32 size ) override;
	virtual IIndexBufferRef CreateIndexBuffer( void* data, uint32 size ) override;

	virtual uint32 CreatePipelineState() override;

	virtual void UpdateConstantBuffer( const struct ConstantBuffer& constBuffer ) override;

	virtual void RunCS() override;

private:
	void SetPipelineState( ID3D12PipelineState* pso, ID3D12RootSignature* rootSignature );

	ID3D12RootSignature* CreateRootSignature();
	ID3D12PipelineState* CreatePipelineState( ID3D12RootSignature* rootSignature );
	ID3D12RootSignature* CreateRootSignature1();
	ID3D12PipelineState* CreatePipelineState1( ID3D12RootSignature* rootSignature );

private:
	HWND _hWnd;
	int32 _screenWidth;
	int32 _screenHeight;

	struct ID3D12Device*	_device;
	struct IDXGISwapChain*	_swapChain;
	ID3D12DescriptorHeap* _rtvHeap;
	ID3D12DescriptorHeap* _dsvHeap;
	ID3D12DescriptorHeap* _cbvHeap;
	ID3D12DescriptorHeap* _uavHeap;

	struct ID3D12Debug*		_debugInterface;
	struct ID3D12InfoQueue* _debugInfoQueue;

	std::unordered_map<enum D3D12_COMMAND_LIST_TYPE, CommandQueueContext> _cmdQueueCtx;

	std::array<SwapChainBufferContext, SWAPCHAIN_BUFFER_COUNT> _swapChainBuffers;
	std::array<SwapChainBufferContext, SWAPCHAIN_BUFFER_COUNT>::iterator _swapChainBufferIter;

	std::unordered_map<uint32, std::tuple<ID3D12RootSignature*, ID3D12PipelineState*>> _pipelineStateCache;
	// temp
	ID3D12Resource* buffer[ 3 ];
	//std::unordered_map<uint32, ID3D12Resource*> _constBufferCache;
};
