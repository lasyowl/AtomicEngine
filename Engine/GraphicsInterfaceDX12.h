#pragma once

#include "GraphicsInterface.h"

#include <d3d12.h>
#include <dxgi.h>

class CRenderObject;

struct SGraphicsInitParamDX12 : public SGraphicsInitParam
{
	HWND WindowHandle;
};

enum
{
	SWAPCHAIN_BUFFERCOUNT = 3,
	COMMANDLIST_PER_QUEUE_COUNT = 3,
};

struct SDX12SwapChainBufferContext
{
	ID3D12Resource* RenderTarget;
	uint32 BufferIndex;
	size_t RenderTargetHandlePtr;
};

class CDX12CommandQueueSet
{
public:
	void ItinerateCommandList();
	void ExecuteCommandList( ID3D12CommandList* CmdList );

public:
	ID3D12CommandQueue* CmdQueue;

	ID3D12CommandAllocator* Allocator;

	std::array<ID3D12GraphicsCommandList*, COMMANDLIST_PER_QUEUE_COUNT> CmdLists;
	std::array<ID3D12GraphicsCommandList*, COMMANDLIST_PER_QUEUE_COUNT>::iterator CmdListIter;

	ID3D12Fence* Fence;
	HANDLE FenceEventHandle;
	uint64 FenceValue;
};

/** Derived interface of DirectX12 */
class CGraphicsInterfaceDX12 : public IGraphicsInterface
{
public:
	CGraphicsInterfaceDX12();

	virtual void InitGraphics( const SGraphicsInitParam& InParam ) override;
	virtual void BeginFrame() override;
	virtual void EndFrame() override;

	virtual void SetPipelineState( IPipelineStateRef& InPipelineState ) override;
	virtual void Render( const CRenderObject& InRenderObject ) override;
	virtual void FlushPipelineState() override;

public:
	virtual IPipelineStateRef CreatePipelineState() override;

	virtual IVertexBufferContextRef CreateVertexBuffer( void* Data, uint64 SizeInByte ) override;
	virtual IIndexBufferContextRef CreateIndexBuffer( void* Data, uint64 SizeInByte ) override;

private:
	void SetWindowParams( const SGraphicsInitParamDX12& InParam );

	void CreateDebugLayer();
	void CreateDevice();
	void CreateDebugInfoQueue();
	void CreateCommandQueuesAndLists();
	void CreateSwapChain();
	void CreateDescriptorHeap();
	void CreateRenderTargets();

	template<typename TBufferContext>
	std::shared_ptr<TBufferContext> CreateBuffer( void* Data, uint64 SizeInByte );

	void CopyMemoryToBuffer( ID3D12Resource* Buffer, void* Data, uint64 SizeInByte );

private:
	ID3D12Device* Device;
	IDXGISwapChain* SwapChain;
	ID3D12DescriptorHeap* DescHeap;

	ID3D12Debug* DebugInterface;
	ID3D12InfoQueue* DebugInfoQueue;

	/** Key : D3D12_COMMAND_LIST_TYPE */
	std::unordered_map<D3D12_COMMAND_LIST_TYPE, CDX12CommandQueueSet> CmdQueueSets;

	std::array<SDX12SwapChainBufferContext, SWAPCHAIN_BUFFERCOUNT> SwapChainBuffers;
	std::array<SDX12SwapChainBufferContext, SWAPCHAIN_BUFFERCOUNT>::iterator SwapChainBufferIter;

	D3D12_VIEWPORT WindowViewport;
	D3D12_RECT WindowScissorRect;
	HWND WindowHandle;
};