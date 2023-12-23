#pragma once

#include "EngineEssential.h"
#include "GPI.h"

constexpr int32 CMD_LIST_PER_QUEUE_COUNT = 3;
constexpr int32 SWAPCHAIN_BUFFER_COUNT = 3;

struct CommandQueueContext
{
public:
	//void ItinerateCommandList();
	//void ExecuteCommandList( ID3D12CommandList* CmdList );

public:
	struct ID3D12CommandQueue* cmdQueue;

	struct ID3D12CommandAllocator* allocator;

	std::array<struct ID3D12GraphicsCommandList*, CMD_LIST_PER_QUEUE_COUNT> cmdLists;
	std::array<struct ID3D12GraphicsCommandList*, CMD_LIST_PER_QUEUE_COUNT>::iterator cmdListIter;

	struct ID3D12Fence* fence;
	HANDLE fenceEventHandle;
	uint64 fenceValue;
};

struct SwapChainBufferContext
{
	struct ID3D12Resource* RenderTarget;
	uint32 BufferIndex;
	size_t RenderTargetHandlePtr;
};

class GPI_DX12 : public IGPI
{
public:
	GPI_DX12( const HWND hWnd, const int32 screenWidth, const int32 screenHeight );

	virtual void Initialize() override;

private:
	HWND _hWnd;
	int32 _screenWidth;
	int32 _screenHeight;

	struct ID3D12Device*	_device;
	struct IDXGISwapChain*	_swapChain;
	struct ID3D12DescriptorHeap* _descHeap;

	struct ID3D12Debug*		_debugInterface;
	struct ID3D12InfoQueue* _debugInfoQueue;

	std::unordered_map<enum D3D12_COMMAND_LIST_TYPE, CommandQueueContext> _cmdQueueCtx;

	std::array<SwapChainBufferContext, SWAPCHAIN_BUFFER_COUNT> _swapChainBuffers;
	std::array<SwapChainBufferContext, SWAPCHAIN_BUFFER_COUNT>::iterator _swapChainBufferIter;

};