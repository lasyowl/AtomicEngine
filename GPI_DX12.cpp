#include "stdafx.h"
#include "GPI_DX12.h"
#include "EngineDefine.h"
#include "AtomicEngine.h"
#include "GPIConstantBuffer.h"
#include "DebugUtil.h"
#include "RawImage.h"
#include "AssetLoader.h"
#include "GPIShader_DX12.h"
#include "GPIPipeline_DX12.h"

#include <d3d12.h>
#include <d3dcommon.h>
#include <dxgi.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <comdef.h>
#include <fstream>

#define CHECK_HRESULT( hr, msg ) \
	if(FAILED (hr)) {\
		AEMessageBox( msg );\
	}

size_t descHeapSize[ D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES ];

constexpr D3D12_VIEWPORT ToDX12Viewport( uint32 width, uint32 height )
{
	return D3D12_VIEWPORT{ 0, 0, ( float )width, ( float )height, 0, 1 };
}

constexpr D3D12_RECT ToDX12Rect( uint32 width, uint32 height )
{
	return D3D12_RECT{ 0, 0, ( int32 )width, ( int32 )height };
}

uint32 GetPipelineStateHash( const GPIPipelineStateDesc& pipelineDesc )
{
	// todo : make complicated? dont remove without consideration
	return pipelineDesc.hash;
}

const char* ShaderTypeToString( EShaderType type )
{
	switch( type )
	{
		case EShaderType::ShaderType_VertexShader:	return "vs_5_0";
		case EShaderType::ShaderType_PixelShader:	return "ps_5_0";
		case EShaderType::ShaderType_ComputeShader:	return "cs_5_0";
	}

	return nullptr;
}

DXGI_FORMAT TranslateBufferFormat( const EGPIBufferFormat bufferFormat )
{
	switch( bufferFormat )
	{
		case GPIBufferFormat_B8G8R8A8:			return DXGI_FORMAT_B8G8R8A8_UNORM;
		case GPIBufferFormat_B8G8R8A8_SRGB:		return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
		case GPIBufferFormat_R32G32_Float:		return DXGI_FORMAT_R32G32_FLOAT;
		case GPIBufferFormat_R32G32B32_Float:	return DXGI_FORMAT_R32G32B32_FLOAT;
	}

	return DXGI_FORMAT_UNKNOWN;
}

D3D12_INPUT_ELEMENT_DESC TranslateInputDesc( const GPIPipelineInputDesc& inputDesc )
{
	D3D12_INPUT_ELEMENT_DESC d3dInputDesc =
	{
		inputDesc.semanticName.c_str(),
		0,
		TranslateBufferFormat( inputDesc.format ),
		inputDesc.inputSlot,
		0,
		inputDesc.inputClass == GPIInputClass_PerInstance ? D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA : D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
		0
	};

	return d3dInputDesc;
}

std::vector<D3D_SHADER_MACRO> TranslateShaderMacros( const std::vector<GPIShaderMacro>& inMacros )
{
	std::vector<D3D_SHADER_MACRO> macros;
	macros.resize( inMacros.size() + 1 );
	for( uint32 index = 0; index < inMacros.size(); ++index )
	{
		macros[ index ].Name = inMacros[ index ].name.c_str();
		macros[ index ].Definition = inMacros[ index ].value.c_str();
	}
	macros[ inMacros.size() ] = {};

	return macros;
}

void TransitionResource( ID3D12GraphicsCommandList* cmdList, ID3D12Resource* resource, D3D12_RESOURCE_STATES stateBefore, D3D12_RESOURCE_STATES stateAfter )
{
	D3D12_RESOURCE_BARRIER barrier;
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = resource;
	barrier.Transition.StateBefore = stateBefore;
	barrier.Transition.StateAfter = stateAfter;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	cmdList->ResourceBarrier( 1, &barrier );
}

void WaitForFence( CommandQueueContext& cmdQueueCtx )
{
	ID3D12Fence* fence = cmdQueueCtx.fence;
	HANDLE fenceEventHandle = cmdQueueCtx.fenceEventHandle;
	uint64& fenceValue = cmdQueueCtx.fenceValue;

	CHECK_HRESULT( cmdQueueCtx.cmdQueue->Signal( fence, ++fenceValue ), L"Failed to signal command queue." );

	uint64 completeValue = fence->GetCompletedValue();
	if( completeValue != fenceValue )
	{
		fence->SetEventOnCompletion( fenceValue, fenceEventHandle );
		WaitForSingleObject( fenceEventHandle, INFINITE );
	}
}

/**
 * Parameterized resource desc getter for vertex buffer
 * @param width : size for 1/2/3d buffer
 * @param height : size for 2/3d buffer
 * @param depth : size for 3d buffer
 * @return DirectX12 resource descriptor
 */
constexpr D3D12_RESOURCE_DESC GetVertexBufferDesc( D3D12_RESOURCE_FLAGS flags, uint64 width, uint32 height = 1, uint16 depth = 1 )
{
	D3D12_RESOURCE_DESC bufferDesc{};
	bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	bufferDesc.Alignment = 0;
	bufferDesc.Width = width;
	bufferDesc.Height = height;
	bufferDesc.DepthOrArraySize = depth;
	bufferDesc.MipLevels = 1;
	bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
	bufferDesc.SampleDesc.Count = 1;
	bufferDesc.SampleDesc.Quality = 0;
	bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	bufferDesc.Flags = flags;

	return bufferDesc;
}

constexpr D3D12_RESOURCE_DESC GetDepthStencilBufferDesc( uint64 width, uint32 height )
{
	D3D12_RESOURCE_DESC bufferDesc{};
	bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	bufferDesc.Alignment = 0;
	bufferDesc.Width = width;
	bufferDesc.Height = height;
	bufferDesc.DepthOrArraySize = 1;
	bufferDesc.MipLevels = 1;
	bufferDesc.Format = DXGI_FORMAT_D32_FLOAT;
	bufferDesc.SampleDesc.Count = 1;
	bufferDesc.SampleDesc.Quality = 0;
	bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	bufferDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	return bufferDesc;
}

constexpr D3D12_RESOURCE_DESC GetGBufferDesc( uint64 width, uint32 height )
{
	D3D12_RESOURCE_DESC bufferDesc{};
	bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	bufferDesc.Alignment = 0;
	bufferDesc.Width = width;
	bufferDesc.Height = height;
	bufferDesc.DepthOrArraySize = 1;
	bufferDesc.MipLevels = 1;
	bufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	bufferDesc.SampleDesc.Count = 1;
	bufferDesc.SampleDesc.Quality = 0;
	bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	bufferDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

	return bufferDesc;
}

constexpr D3D12_RESOURCE_DESC GetTextureBufferDesc( uint64 width, uint32 height )
{
	D3D12_RESOURCE_DESC bufferDesc{};
	bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	bufferDesc.Alignment = 0;
	bufferDesc.Width = width;
	bufferDesc.Height = height;
	bufferDesc.DepthOrArraySize = 1;
	bufferDesc.MipLevels = 1;
	bufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	bufferDesc.SampleDesc.Count = 1;
	bufferDesc.SampleDesc.Quality = 0;
	bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	bufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	return bufferDesc;
}

constexpr D3D12_HEAP_PROPERTIES HeapProperties( D3D12_HEAP_TYPE heapType )
{
	D3D12_HEAP_PROPERTIES heapProp{};
	heapProp.Type = heapType;
	heapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapProp.CreationNodeMask = 1;
	heapProp.VisibleNodeMask = 1;

	return heapProp;
}

ID3D12DescriptorHeap* CreateDescriptorHeap( ID3D12Device* device, int32 numDesc, D3D12_DESCRIPTOR_HEAP_TYPE descType, D3D12_DESCRIPTOR_HEAP_FLAGS descFlag )
{
	ID3D12DescriptorHeap* descHeap;

	D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc{};
	descHeapDesc.NumDescriptors = numDesc;
	descHeapDesc.Type = descType;
	descHeapDesc.Flags = descFlag;

	CHECK_HRESULT( device->CreateDescriptorHeap( &descHeapDesc, IID_PPV_ARGS( &descHeap ) ),
				   L"Failed to create descriptor heap." );

	return descHeap;
}

constexpr D3D12_RENDER_TARGET_VIEW_DESC GetRTVDesc()
{
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	rtvDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2D.MipSlice = 0;
	rtvDesc.Texture2D.PlaneSlice = 0;

	return rtvDesc;
}

constexpr D3D12_DEPTH_STENCIL_VIEW_DESC GetDSVDesc()
{
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

	return dsvDesc;
}

void CopyMemoryToBuffer( ID3D12Resource* buffer, void* data, uint64 size )
{
	void* virtualMem;
	buffer->Map( 0, nullptr, &virtualMem );
	::memcpy( virtualMem, data, size );
	buffer->Unmap( 0, nullptr );
}

ID3D12Resource* CreateBuffer( ID3D12Device* device, CommandQueueContext& cmdQueueCtx, void* data, uint64 size )
{
	ID3D12GraphicsCommandList* cmdList = *cmdQueueCtx.cmdListIter;
	ID3D12CommandAllocator* cmdAllocator = cmdQueueCtx.allocator;

	D3D12_RESOURCE_DESC uploadBufferDesc = GetVertexBufferDesc( D3D12_RESOURCE_FLAG_NONE, size );
	D3D12_RESOURCE_DESC outbufferDesc = GetVertexBufferDesc( D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, size );
	D3D12_HEAP_PROPERTIES uploadHeapProp = HeapProperties( D3D12_HEAP_TYPE_UPLOAD );
	D3D12_HEAP_PROPERTIES defaultHeapProp = HeapProperties( D3D12_HEAP_TYPE_DEFAULT );

	// Create upload buffer on CPU
	ID3D12Resource* uploadBuffer;
	CHECK_HRESULT( device->CreateCommittedResource( &uploadHeapProp,
				   D3D12_HEAP_FLAG_NONE,
				   &uploadBufferDesc,
				   D3D12_RESOURCE_STATE_GENERIC_READ,
				   nullptr,
				   IID_PPV_ARGS( &uploadBuffer ) ),
				   L"Failed to create upload buffer." );

	ID3D12Resource* outBuffer;
	CHECK_HRESULT( device->CreateCommittedResource( &defaultHeapProp,
				   D3D12_HEAP_FLAG_NONE,
				   &outbufferDesc,
				   D3D12_RESOURCE_STATE_COPY_DEST,
				   nullptr,
				   IID_PPV_ARGS( &outBuffer ) ),
				   L"Failed to create output buffer." );

	if( data )
	{
		CopyMemoryToBuffer( uploadBuffer, data, size );
	}

	CHECK_HRESULT( cmdAllocator->Reset(), L"Failed to reset command allocator." );
	CHECK_HRESULT( cmdList->Reset( cmdAllocator, nullptr ), L"Failed to reset command list." );

	cmdList->CopyBufferRegion( outBuffer, 0, uploadBuffer, 0, size );

	D3D12_RESOURCE_BARRIER barrier;
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = outBuffer;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COMMON;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	cmdList->ResourceBarrier( 1, &barrier );

	CHECK_HRESULT( cmdList->Close(), L"Failed to close command list." );

	ID3D12CommandList* cmdListInterface = cmdList;
	cmdQueueCtx.cmdQueue->ExecuteCommandLists( 1, &cmdListInterface );

	WaitForFence( cmdQueueCtx );

	return outBuffer;
}

ID3D12Resource* CreateGBuffer( ID3D12Device* device, uint32 width, uint32 height )
{
	D3D12_RESOURCE_DESC outbufferDesc = GetGBufferDesc( width, height );
	D3D12_HEAP_PROPERTIES defaultHeapProp = HeapProperties( D3D12_HEAP_TYPE_DEFAULT );

	D3D12_CLEAR_VALUE clearValue{};
	clearValue.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	clearValue.Color[ 0 ] = 1;
	clearValue.Color[ 1 ] = 1;
	clearValue.Color[ 2 ] = 1;
	clearValue.Color[ 3 ] = 1;

	ID3D12Resource* outBuffer;
	CHECK_HRESULT( device->CreateCommittedResource( &defaultHeapProp,
				   D3D12_HEAP_FLAG_NONE,
				   &outbufferDesc,
				   D3D12_RESOURCE_STATE_RENDER_TARGET,
				   &clearValue,
				   IID_PPV_ARGS( &outBuffer ) ),
				   L"Failed to create output buffer." );

	return outBuffer;
}

ID3D12Resource* CreateDepthStencilBuffer( ID3D12Device* device, uint32 width, uint32 height )
{
	D3D12_RESOURCE_DESC outbufferDesc = GetDepthStencilBufferDesc( width, height );
	D3D12_HEAP_PROPERTIES defaultHeapProp = HeapProperties( D3D12_HEAP_TYPE_DEFAULT );

	D3D12_CLEAR_VALUE clearValue{};
	clearValue.Format = DXGI_FORMAT_D32_FLOAT;
	clearValue.DepthStencil.Depth = 1.0f;
	clearValue.DepthStencil.Stencil = 0;

	ID3D12Resource* outBuffer;
	CHECK_HRESULT( device->CreateCommittedResource( &defaultHeapProp,
				   D3D12_HEAP_FLAG_NONE,
				   &outbufferDesc,
				   D3D12_RESOURCE_STATE_DEPTH_WRITE,
				   &clearValue,
				   IID_PPV_ARGS( &outBuffer ) ),
				   L"Failed to create output buffer." );

	return outBuffer;
}

ID3D12Resource* CreateTextureBuffer( ID3D12Device* device, uint32 width, uint32 height )
{
	D3D12_RESOURCE_DESC outbufferDesc = GetTextureBufferDesc( width, height );
	D3D12_HEAP_PROPERTIES defaultHeapProp = HeapProperties( D3D12_HEAP_TYPE_DEFAULT );

	ID3D12Resource* outBuffer;
	CHECK_HRESULT( device->CreateCommittedResource( &defaultHeapProp,
				   D3D12_HEAP_FLAG_NONE,
				   &outbufferDesc,
				   D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE,
				   nullptr,
				   IID_PPV_ARGS( &outBuffer ) ),
				   L"Failed to create output buffer." );

	return outBuffer;
}

void UpdateBuffer( ID3D12Device* device, CommandQueueContext& cmdQueueCtx, ID3D12Resource* outBuffer, void* data, uint64 size )
{
	ID3D12GraphicsCommandList* cmdList = *cmdQueueCtx.cmdListIter;
	ID3D12CommandAllocator* cmdAllocator = cmdQueueCtx.allocator;

	D3D12_RESOURCE_DESC resourceDesc = GetVertexBufferDesc( D3D12_RESOURCE_FLAG_NONE, size );
	D3D12_HEAP_PROPERTIES uploadHeapProp = HeapProperties( D3D12_HEAP_TYPE_UPLOAD );

	// Create upload buffer on CPU
	ID3D12Resource* uploadBuffer;
	CHECK_HRESULT( device->CreateCommittedResource( &uploadHeapProp,
				   D3D12_HEAP_FLAG_NONE,
				   &resourceDesc,
				   D3D12_RESOURCE_STATE_GENERIC_READ,
				   nullptr,
				   IID_PPV_ARGS( &uploadBuffer ) ),
				   L"Failed to create upload buffer." );

	CopyMemoryToBuffer( uploadBuffer, data, size );

	CHECK_HRESULT( cmdAllocator->Reset(), L"Failed to reset command allocator." );
	CHECK_HRESULT( cmdList->Reset( cmdAllocator, nullptr ), L"Failed to reset command list." );

	cmdList->CopyBufferRegion( outBuffer, 0, uploadBuffer, 0, size );

	D3D12_RESOURCE_BARRIER barrier;
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = outBuffer;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COMMON;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	cmdList->ResourceBarrier( 1, &barrier );

	CHECK_HRESULT( cmdList->Close(), L"Failed to close command list." );

	ID3D12CommandList* cmdListInterface = cmdList;
	cmdQueueCtx.cmdQueue->ExecuteCommandLists( 1, &cmdListInterface );

	WaitForFence( cmdQueueCtx );

	uploadBuffer->Release();
}

void UpdateTexture( ID3D12Device* device, CommandQueueContext& cmdQueueCtx, ID3D12Resource* outBuffer, void* data, uint32 width, uint32 height )
{
	ID3D12GraphicsCommandList* cmdList = *cmdQueueCtx.cmdListIter;
	ID3D12CommandAllocator* cmdAllocator = cmdQueueCtx.allocator;

	D3D12_RESOURCE_DESC resourceDesc = GetVertexBufferDesc( D3D12_RESOURCE_FLAG_NONE, width * height * sizeof( uint32 ) );// outBuffer->GetDesc();
	D3D12_HEAP_PROPERTIES heapProp = HeapProperties( D3D12_HEAP_TYPE_UPLOAD );

	// Create upload buffer on CPU
	ID3D12Resource* uploadBuffer;
	CHECK_HRESULT( device->CreateCommittedResource( &heapProp,
				   D3D12_HEAP_FLAG_NONE,
				   &resourceDesc,
				   D3D12_RESOURCE_STATE_GENERIC_READ,
				   nullptr,
				   IID_PPV_ARGS( &uploadBuffer ) ),
				   L"Failed to create upload buffer." );

	CopyMemoryToBuffer( uploadBuffer, data, width * height * sizeof( uint32 ) );

	CHECK_HRESULT( cmdAllocator->Reset(), L"Failed to reset command allocator." );
	CHECK_HRESULT( cmdList->Reset( cmdAllocator, nullptr ), L"Failed to reset command list." );

	TransitionResource( cmdList, outBuffer, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST );

	uint64 requiredSize = 0;
	D3D12_PLACED_SUBRESOURCE_FOOTPRINT layout;
	uint32 numRow;
	uint64 rowSizesInBytes;

	D3D12_RESOURCE_DESC outDesc = outBuffer->GetDesc();
	device->GetCopyableFootprints( &outDesc, 0, 1, 0, &layout, &numRow, &rowSizesInBytes, &requiredSize );

	D3D12_TEXTURE_COPY_LOCATION srcLocation{};
	srcLocation.pResource = uploadBuffer;
	srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
	srcLocation.PlacedFootprint = layout;

	D3D12_TEXTURE_COPY_LOCATION dstLocation{};
	dstLocation.pResource = outBuffer;
	dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
	dstLocation.SubresourceIndex = 0;

	cmdList->CopyTextureRegion( &dstLocation, 0, 0, 0, &srcLocation, nullptr );

	TransitionResource( cmdList, outBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE );

	CHECK_HRESULT( cmdList->Close(), L"Failed to close command list." );

	ID3D12CommandList* cmdListInterface = cmdList;
	cmdQueueCtx.cmdQueue->ExecuteCommandLists( 1, &cmdListInterface );
	
	WaitForFence( cmdQueueCtx );

	uploadBuffer->Release();
}

///////////////////////////////////////
// GPI_DX12
///////////////////////////////////////
GPI_DX12::GPI_DX12( const HWND hWnd, const int32 screenWidth, const int32 screenHeight )
	: _device( nullptr )
	, _swapChain( nullptr )
	, _swapChainIndex( 0 )
	, _swapChainBuffers( {} )
	, _rtvHeap( nullptr )
	, _dsvHeap( nullptr )
	, _uavHeap( nullptr )
	, _debugInterface( nullptr )
	, _debugInfoQueue( nullptr )
	, _hWnd( hWnd )
	, _screenWidth( screenWidth )
	, _screenHeight( screenHeight )
{}

void GPI_DX12::Initialize()
{
	/* Cretate and enable debug layer */
	CHECK_HRESULT( D3D12GetDebugInterface( IID_PPV_ARGS( &_debugInterface ) ), L"Failed to create debug layer." );
	_debugInterface->EnableDebugLayer();

	/* Create device */
	CHECK_HRESULT( D3D12CreateDevice( nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS( &_device ) ), L"Failed to create device." );

	/* Create command queue and lists */
	{
		D3D12_COMMAND_LIST_TYPE cmdListTypes[] = {
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			D3D12_COMMAND_LIST_TYPE_COMPUTE,
			D3D12_COMMAND_LIST_TYPE_COPY
		};

		for( D3D12_COMMAND_LIST_TYPE type : cmdListTypes )
		{
			CommandQueueContext& cmdQueueCtx = _cmdQueueCtx[ type ];

			D3D12_COMMAND_QUEUE_DESC queueDesc{};
			queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
			queueDesc.Type = type;

			CHECK_HRESULT( _device->CreateCommandQueue( &queueDesc, IID_PPV_ARGS( &cmdQueueCtx.cmdQueue ) ),
						   L"Failed to create command queue." );
			CHECK_HRESULT( _device->CreateCommandAllocator( queueDesc.Type, IID_PPV_ARGS( &cmdQueueCtx.allocator ) ),
						   L"Failed to create command allocator." );

			for( int32 Index = 0; Index < CMD_LIST_PER_QUEUE_COUNT; ++Index )
			{
				ID3D12GraphicsCommandList*& cmdList = cmdQueueCtx.cmdLists[ Index ];

				CHECK_HRESULT( _device->CreateCommandList( 0, queueDesc.Type, cmdQueueCtx.allocator, nullptr, IID_PPV_ARGS( &cmdList ) ),
							   L"Failed to create command list." );
				CHECK_HRESULT( cmdList->Close(),
							   L"Failed to close command list." );
			}

			CHECK_HRESULT( _device->CreateFence( 0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS( &cmdQueueCtx.fence ) ),
						   L"Failed to create fence." );

			cmdQueueCtx.fenceEventHandle = CreateEvent( nullptr, FALSE, FALSE, nullptr );
			cmdQueueCtx.fenceValue = 0;
			cmdQueueCtx.cmdListIter = cmdQueueCtx.cmdLists.begin();
		}
	}

	/* Create swapchain */
	{
		CommandQueueContext& cmdQueueCtx = _cmdQueueCtx[ D3D12_COMMAND_LIST_TYPE_DIRECT ];

		DXGI_SWAP_CHAIN_DESC swapChainDesc{};
		swapChainDesc.BufferCount = SWAPCHAIN_COUNT;
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferDesc.Width = _screenWidth;
		swapChainDesc.BufferDesc.Height = _screenHeight;
		swapChainDesc.OutputWindow = _hWnd;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.Windowed = true;

		IDXGIFactory6* dxgiFactory;
		CHECK_HRESULT( CreateDXGIFactory1( IID_PPV_ARGS( &dxgiFactory ) ),
					   L"Failed to create dxgi factory." );
		CHECK_HRESULT( dxgiFactory->CreateSwapChain( cmdQueueCtx.cmdQueue, &swapChainDesc, &_swapChain ),
					   L"Failed to create swapchain." );
	}

	/* Get constant values */
	for( uint32 index = 0; index < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++index )
	{
		descHeapSize[ index ] = _device->GetDescriptorHandleIncrementSize( ( D3D12_DESCRIPTOR_HEAP_TYPE )index );
	}

	/* Create depth buffer */
	{
		_dsvHeap = CreateDescriptorHeap( _device, 1, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE );

		constexpr D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = GetDSVDesc();

		D3D12_CPU_DESCRIPTOR_HANDLE dsvDescHandle = _dsvHeap->GetCPUDescriptorHandleForHeapStart();

		_depthStencilBuffer = CreateDepthStencilBuffer( _device, 1920, 1080 );

		_device->CreateDepthStencilView( _depthStencilBuffer, &dsvDesc, dsvDescHandle );
	}

	/* Rendertargets */
	{
		_rtvHeap = CreateDescriptorHeap( _device, SWAPCHAIN_COUNT + 4, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE );

		constexpr D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = GetRTVDesc();

		D3D12_CPU_DESCRIPTOR_HANDLE rtvDescHandle = _rtvHeap->GetCPUDescriptorHandleForHeapStart();

		for( uint32 index = 0; index < _swapChainBuffers.size(); ++index )
		{
			ID3D12Resource*& swapChainBuffer = _swapChainBuffers[ index ];

			CHECK_HRESULT( _swapChain->GetBuffer( index, IID_PPV_ARGS( &swapChainBuffer ) ), L"Failed to get swapchain buffer." );

			_device->CreateRenderTargetView( swapChainBuffer, &rtvDesc, rtvDescHandle );

			rtvDescHandle.ptr += descHeapSize[ D3D12_DESCRIPTOR_HEAP_TYPE_RTV ];
		}

		for( ID3D12Resource*& gBuffer : _gBuffers )
		{
			gBuffer = CreateGBuffer( _device, 1920, 1080 );

			_device->CreateRenderTargetView( gBuffer, &rtvDesc, rtvDescHandle );

			rtvDescHandle.ptr += descHeapSize[ D3D12_DESCRIPTOR_HEAP_TYPE_RTV ];
		}
	}

	/* Constant buffers */
	{
		_rvHeap = CreateDescriptorHeap( _device, 1 + 4/*number of GBuffers*/, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);

		float data[ 64 ] = { 1, 1, 1, 1 };
		_constBuffer = CreateBuffer( _device, _cmdQueueCtx[ D3D12_COMMAND_LIST_TYPE_COPY ], data, 256 );

		D3D12_CONSTANT_BUFFER_VIEW_DESC cbDesc{};
		cbDesc.BufferLocation = _constBuffer->GetGPUVirtualAddress();
		cbDesc.SizeInBytes = 256;

		D3D12_CPU_DESCRIPTOR_HANDLE rvDescHandle = _rvHeap->GetCPUDescriptorHandleForHeapStart();
		_device->CreateConstantBufferView( &cbDesc, rvDescHandle );

		rvDescHandle.ptr += descHeapSize[ D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV ];

		{
			for( uint32 index = 0; index < 3; ++index )
			{
				ID3D12Resource*& gBuffer = _gBuffers[ index ];

				D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
				srvDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
				srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
				srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
				srvDesc.Texture2D.MipLevels = 1;

				_device->CreateShaderResourceView( gBuffer, &srvDesc, rvDescHandle );

				rvDescHandle.ptr += descHeapSize[ D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV ];
			}

			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
			srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvDesc.Texture2D.MipLevels = 1;

			_device->CreateShaderResourceView( _depthStencilBuffer, &srvDesc, rvDescHandle );
		}

		///* Create shader resource buffers, !!temp!! */
		//{
		//	rawImage = AssetLoader::LoadRawImage( "Resource/test.png" );
		//	textureBuffer = CreateTextureBuffer( _device, rawImage->width, rawImage->height );
		//	UpdateTexture( _device, _cmdQueueCtx[ D3D12_COMMAND_LIST_TYPE_DIRECT ], textureBuffer, rawImage->data.data(), rawImage->width, rawImage->height);

		//	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
		//	srvDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
		//	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		//	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		//	srvDesc.Texture2D.MipLevels = 1;

		//	_device->CreateShaderResourceView( textureBuffer, &srvDesc, rvDescHandle );
		//}

		_cbvHeap = CreateDescriptorHeap( _device, 10, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE );
		_srvHeap = CreateDescriptorHeap( _device, 10, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE );
	}

	/* Unordered access view */
	{
		_uavHeap = CreateDescriptorHeap( _device, 4, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE );

		D3D12_CPU_DESCRIPTOR_HANDLE uavDescHandle = _uavHeap->GetCPUDescriptorHandleForHeapStart();

		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
		uavDesc.Format = DXGI_FORMAT_R32_UINT;
		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
		uavDesc.Buffer.FirstElement = 0;
		uavDesc.Buffer.CounterOffsetInBytes = 0;
		uavDesc.Buffer.NumElements = 1920 * 1080;
		uavDesc.Buffer.StructureByteStride = 0;
		uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

		for( ID3D12Resource*& gBuffer : _gBuffers )
		{
			_device->CreateUnorderedAccessView( gBuffer, nullptr, &uavDesc, uavDescHandle );

			uavDescHandle.ptr += descHeapSize[ D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV ];
		}
	}
}

void GPI_DX12::BeginFrame()
{
	CommandQueueContext& cmdQueueCtx = _cmdQueueCtx[ D3D12_COMMAND_LIST_TYPE_DIRECT ];
	ID3D12CommandAllocator* cmdAllocator = cmdQueueCtx.allocator;
	ID3D12GraphicsCommandList* cmdList = *cmdQueueCtx.cmdListIter;

	CHECK_HRESULT( cmdAllocator->Reset(), L"Failed to reset command allocator." );
	CHECK_HRESULT( cmdList->Reset( cmdAllocator, nullptr ), L"Failed to reset command list." );

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = _rtvHeap->GetCPUDescriptorHandleForHeapStart();
	rtvHandle.ptr += _swapChainIndex * descHeapSize[ D3D12_DESCRIPTOR_HEAP_TYPE_RTV ];
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = _dsvHeap->GetCPUDescriptorHandleForHeapStart();
	cmdList->OMSetRenderTargets( 1, &rtvHandle, true, &dsvHandle );

	D3D12_VIEWPORT viewport = ToDX12Viewport( _windowWidth, _windowHeight );
	D3D12_RECT scissorRect = ToDX12Rect( _windowWidth, _windowHeight );
	cmdList->RSSetViewports( 1, &viewport );
	cmdList->RSSetScissorRects( 1, &scissorRect );

	// State transition of back buffer
	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Transition.pResource = _swapChainBuffers[ _swapChainIndex ];
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	cmdList->ResourceBarrier( 1, &barrier );

	static float clearColor[] = {
		0.242f, 0.242f, 0.442f, 1
	};

	cmdList->ClearDepthStencilView( dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr );
	cmdList->ClearRenderTargetView( rtvHandle, clearColor, 0, nullptr );

	uint32 clearValue[ 4 ]{};
	D3D12_GPU_DESCRIPTOR_HANDLE gBufferGPUHandle = _uavHeap->GetGPUDescriptorHandleForHeapStart();
	D3D12_CPU_DESCRIPTOR_HANDLE gBufferCPUHandle = _uavHeap->GetCPUDescriptorHandleForHeapStart();
	for( uint32 index = 0; index < 4; ++index )
	{
		TransitionResource( cmdList, _gBuffers[ index ], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_UNORDERED_ACCESS );

		cmdList->ClearUnorderedAccessViewUint( gBufferGPUHandle, gBufferCPUHandle, _gBuffers[ index ], clearValue, 0, nullptr );

		TransitionResource( cmdList, _gBuffers[ index ], D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_RENDER_TARGET );

		gBufferGPUHandle.ptr += descHeapSize[ D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV ];
		gBufferCPUHandle.ptr += descHeapSize[ D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV ];
	}

	CHECK_HRESULT( cmdList->Close(), L"Failed to close command list." );

	ID3D12CommandList* cmdListInterface = cmdList;
	cmdQueueCtx.cmdQueue->ExecuteCommandLists( 1, &cmdListInterface );

	WaitForFence( cmdQueueCtx );
}

void GPI_DX12::EndFrame()
{
	CommandQueueContext& cmdQueueCtx = _cmdQueueCtx[ D3D12_COMMAND_LIST_TYPE_DIRECT ];
	ID3D12CommandAllocator* cmdAllocator = cmdQueueCtx.allocator;
	ID3D12GraphicsCommandList* cmdList = *cmdQueueCtx.cmdListIter;

	CHECK_HRESULT( cmdAllocator->Reset(), L"Failed to reset command allocator." );
	CHECK_HRESULT( cmdList->Reset( cmdAllocator, nullptr ), L"Failed to reset command list." );
	
	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Transition.pResource = _swapChainBuffers[ _swapChainIndex ];
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	cmdList->ResourceBarrier( 1, &barrier );
	
	CHECK_HRESULT( cmdList->Close(), L"Failed to close command list." );
	
	ID3D12CommandList* cmdListInterface = cmdList;
	cmdQueueCtx.cmdQueue->ExecuteCommandLists( 1, &cmdListInterface );

	CHECK_HRESULT( _swapChain->Present( 1, 0 ), L"Failed to present swapchain." );

	WaitForFence( cmdQueueCtx );

	_swapChainIndex = ( _swapChainIndex + 1 ) % SWAPCHAIN_COUNT;

	if( ++cmdQueueCtx.cmdListIter == cmdQueueCtx.cmdLists.end() )
	{
		cmdQueueCtx.cmdListIter = cmdQueueCtx.cmdLists.begin();
	}
}

void GPI_DX12::SetPipelineState( const GPIPipelineStateDesc& desc, const GPIPipeline_DX12& pipeline )
{
	CommandQueueContext& cmdQueueCtx = _cmdQueueCtx[ D3D12_COMMAND_LIST_TYPE_DIRECT ];
	ID3D12CommandAllocator* cmdAllocator = cmdQueueCtx.allocator;
	ID3D12GraphicsCommandList* cmdList = *cmdQueueCtx.cmdListIter;

	CHECK_HRESULT( cmdAllocator->Reset(), L"Failed to reset allocator." );
	CHECK_HRESULT( cmdList->Reset( cmdAllocator, nullptr ), L"Failed to reset command list." );

	cmdList->SetPipelineState( pipeline.pipelineState );
	cmdList->SetGraphicsRootSignature( pipeline.rootSignature );
	cmdList->IASetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = _dsvHeap->GetCPUDescriptorHandleForHeapStart();
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = _rtvHeap->GetCPUDescriptorHandleForHeapStart();
	const size_t rtvDescSize = descHeapSize[ D3D12_DESCRIPTOR_HEAP_TYPE_RTV ];
	rtvHandle.ptr += desc.bRenderSwapChainBuffer ? _swapChainIndex * rtvDescSize : SWAPCHAIN_COUNT * rtvDescSize;
	cmdList->OMSetRenderTargets( desc.numRenderTargets, &rtvHandle, true, &dsvHandle );

	D3D12_VIEWPORT viewport = ToDX12Viewport( _windowWidth, _windowHeight );
	D3D12_RECT scissorRect = ToDX12Rect( _windowWidth, _windowHeight );
	cmdList->RSSetViewports( 1, &viewport );
	cmdList->RSSetScissorRects( 1, &scissorRect );

	cmdList->SetDescriptorHeaps( 1, &_rvHeap );
	cmdList->SetGraphicsRootDescriptorTable( 0, _rvHeap->GetGPUDescriptorHandleForHeapStart() );
	if( pipeline.constBuffer )
	{
		cmdList->SetGraphicsRootConstantBufferView( 1, pipeline.constBuffer->GetGPUVirtualAddress() );
	}
	for( uint32 index = 0; index < min(1, pipeline.resourceBuffers.size()); ++index )
	{
		assert( pipeline.resourceBuffers[ index ] );

		cmdList->SetGraphicsRootShaderResourceView( 2 + index, pipeline.resourceBuffers[ index ]->GetGPUVirtualAddress() );
	}
}

void GPI_DX12::SetPipelineState( const GPIPipelineStateDesc& pipelineDesc )
{
	uint32 pipelineHash = GetPipelineStateHash( pipelineDesc );
	assert( _pipelineCache.contains( pipelineHash ) );

	std::shared_ptr<GPIPipeline_DX12>& pipeline = _pipelineCache[ pipelineHash ];

	SetPipelineState( pipelineDesc, *pipeline.get() );
}

void GPI_DX12::Render( IVertexBuffer* positionBuffer, IVertexBuffer* uvBuffer, IVertexBuffer* normalBuffer, IIndexBuffer* indexBuffer )
{
	CommandQueueContext& cmdQueueCtx = _cmdQueueCtx[ D3D12_COMMAND_LIST_TYPE_DIRECT ];
	ID3D12GraphicsCommandList* cmdList = *cmdQueueCtx.cmdListIter;

	D3D12_VERTEX_BUFFER_VIEW VBViews[ 3 ]{};
	VBViews[ 0 ].BufferLocation = positionBuffer->GetGPUVirtualAddress();
	VBViews[ 0 ].SizeInBytes = ( uint32 )positionBuffer->GetSize();
	VBViews[ 0 ].StrideInBytes = positionBuffer->GetStride();
	if( normalBuffer )
	{
		VBViews[ 1 ].BufferLocation = normalBuffer->GetGPUVirtualAddress();
		VBViews[ 1 ].SizeInBytes = ( uint32 )normalBuffer->GetSize();
		VBViews[ 1 ].StrideInBytes = normalBuffer->GetStride();
	}
	if( uvBuffer )
	{
		VBViews[ 2 ].BufferLocation = uvBuffer->GetGPUVirtualAddress();
		VBViews[ 2 ].SizeInBytes = ( uint32 )uvBuffer->GetSize();
		VBViews[ 2 ].StrideInBytes = uvBuffer->GetStride();
	}

	D3D12_INDEX_BUFFER_VIEW IBView{};
	IBView.BufferLocation = indexBuffer->GetGPUVirtualAddress();
	IBView.SizeInBytes = ( uint32 )indexBuffer->GetSize();
	IBView.Format = DXGI_FORMAT_R32_UINT;

	cmdList->IASetVertexBuffers( 0, 3, VBViews );
	cmdList->IASetIndexBuffer( &IBView );
	cmdList->DrawIndexedInstanced( indexBuffer->GetSize() / sizeof( uint32 ), 1, 0, 0, 0 );
}

void GPI_DX12::FlushPipelineState()
{
	CommandQueueContext& cmdQueueCtx = _cmdQueueCtx[ D3D12_COMMAND_LIST_TYPE_DIRECT ];
	ID3D12GraphicsCommandList* cmdList = *cmdQueueCtx.cmdListIter;

	CHECK_HRESULT( cmdList->Close(), L"Failed to close command list." );

	ID3D12CommandList* cmdListInterface = cmdList;
	cmdQueueCtx.cmdQueue->ExecuteCommandLists( 1, &cmdListInterface );

	WaitForFence( cmdQueueCtx );
}

IVertexBufferRef GPI_DX12::CreateVertexBuffer( void* data, uint32 stride, uint32 size )
{
	ID3D12Resource* buffer = CreateBuffer( _device, _cmdQueueCtx[ D3D12_COMMAND_LIST_TYPE_COPY ], data, size );
	return std::make_shared<VertexBuffer_DX12>( buffer, size, stride );
}

IIndexBufferRef GPI_DX12::CreateIndexBuffer( void* data, uint32 size )
{
	ID3D12Resource* buffer = CreateBuffer( _device, _cmdQueueCtx[ D3D12_COMMAND_LIST_TYPE_COPY ], data, size );
	return std::make_shared<IndexBuffer_DX12>( buffer, size );
}

IVertexBufferRef GPI_DX12::CreateResourceBuffer( void* data, uint32 size )
{
	assert( _resourceBufferCache.size() < 10 );

	ID3D12Resource* resourceBuffer = CreateBuffer( _device, _cmdQueueCtx[ D3D12_COMMAND_LIST_TYPE_COPY ], data, size );
	uint32 bufferHash = _resourceBufferCache.size();
	_resourceBufferCache[ bufferHash ] = resourceBuffer;

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.NumElements = 1;
	//srvDesc.Buffer.StructureByteStride = 0;
	//srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	srvDesc.Buffer.StructureByteStride = 12;

	D3D12_CPU_DESCRIPTOR_HANDLE srvDescHandle = _srvHeap->GetCPUDescriptorHandleForHeapStart();
	srvDescHandle.ptr += ( _resourceBufferCache.size() - 1 ) * descHeapSize[ D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV ];
	_device->CreateShaderResourceView( resourceBuffer, &srvDesc, srvDescHandle );

	return std::make_shared<VertexBuffer_DX12>( resourceBuffer, size, 0 );
}

ID3D12Resource* GPI_DX12::CreateConstantBuffer( void* data, uint32 size )
{
	assert( _constBufferCache.size() < 10 );

	ID3D12Resource* constBuffer = CreateBuffer( _device, _cmdQueueCtx[ D3D12_COMMAND_LIST_TYPE_COPY ], data, size );
	uint32 bufferHash = _constBufferCache.size();
	_constBufferCache[ bufferHash ] = constBuffer;

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbDesc{};
	cbDesc.BufferLocation = constBuffer->GetGPUVirtualAddress();
	cbDesc.SizeInBytes = size;

	D3D12_CPU_DESCRIPTOR_HANDLE cbvDescHandle = _cbvHeap->GetCPUDescriptorHandleForHeapStart();
	cbvDescHandle.ptr += ( _constBufferCache.size() - 1 ) * descHeapSize[ D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV ];
	_device->CreateConstantBufferView( &cbDesc, cbvDescHandle );

	return constBuffer;
}

ID3DBlob* CreateShader( const GPIShaderDesc& shaderDesc )
{
	std::vector<D3D_SHADER_MACRO> macros = TranslateShaderMacros( shaderDesc.macros );

	std::ifstream shaderFile( shaderDesc.file, std::ios_base::in );
	std::string parsedShader = std::string( std::istreambuf_iterator<char>( shaderFile ),
											std::istreambuf_iterator<char>() );

	ID3DBlob* compiledShader;
	ID3DBlob* errorBlob;
	CHECK_HRESULT( D3DCompile( parsedShader.c_str(), parsedShader.size(),
				   "", macros.data(), nullptr,
				   shaderDesc.entry.c_str(), ShaderTypeToString( shaderDesc.type ), 0, 0, &compiledShader, &errorBlob ),
				   L"Shader compilation failed." );
	if( errorBlob )
	{
		std::string errorMsg = ( char* )errorBlob->GetBufferPointer();
		std::wstring wErrorMsg;
		wErrorMsg.assign( errorMsg.begin(), errorMsg.end() );
		AEMessageBox( wErrorMsg );
	}

	return compiledShader;
}

ID3D12RootSignature* CreateGraphicsRootSignature( ID3D12Device* device, const GPIPipelineStateDesc& pipelineDesc )
{
	ID3D12RootSignature* rootSignature;

	D3D12_DESCRIPTOR_RANGE descRange[ 2 ]{};
	descRange[ 0 ].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	descRange[ 0 ].NumDescriptors = 1;
	descRange[ 0 ].BaseShaderRegister = 0;
	descRange[ 1 ].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descRange[ 1 ].NumDescriptors = 4;
	descRange[ 1 ].BaseShaderRegister = 0;
	descRange[ 1 ].OffsetInDescriptorsFromTableStart = 1;

	std::vector<D3D12_ROOT_PARAMETER> rootParams;
	rootParams.resize( 1 + pipelineDesc.numConstantBuffers + pipelineDesc.numResources );
	rootParams[ 0 ].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParams[ 0 ].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParams[ 0 ].DescriptorTable.NumDescriptorRanges = 2;
	rootParams[ 0 ].DescriptorTable.pDescriptorRanges = descRange;

	uint32 rootParamIndex = 1;
	for( uint32 iter = 0; iter < pipelineDesc.numConstantBuffers; ++iter, ++rootParamIndex )
	{
		D3D12_ROOT_DESCRIPTOR rootDesc;
		rootDesc.ShaderRegister = 1 + iter;
		rootDesc.RegisterSpace = 0;

		rootParams[ rootParamIndex ].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		rootParams[ rootParamIndex ].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		rootParams[ rootParamIndex ].Descriptor = rootDesc;
	}

	for( uint32 iter = 0; iter < pipelineDesc.numResources; ++iter, ++rootParamIndex )
	{
		D3D12_ROOT_DESCRIPTOR rootDesc;
		rootDesc.ShaderRegister = 4 + iter;
		rootDesc.RegisterSpace = 0;

		rootParams[ rootParamIndex ].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
		rootParams[ rootParamIndex ].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		rootParams[ rootParamIndex ].Descriptor = rootDesc;
	}

	D3D12_STATIC_SAMPLER_DESC samplerDesc{};
	samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	samplerDesc.ShaderRegister = 0;
	samplerDesc.RegisterSpace = 0;

	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
	rootSignatureDesc.NumParameters = rootParams.size();
	rootSignatureDesc.pParameters = rootParams.data();
	rootSignatureDesc.NumStaticSamplers = 1;
	rootSignatureDesc.pStaticSamplers = &samplerDesc;
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	ID3DBlob* rootBlob;
	ID3DBlob* errorBlob;
	CHECK_HRESULT( D3D12SerializeRootSignature( &rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &rootBlob, &errorBlob ),
				   L"Failed to serialize root signature" );
	if( errorBlob )
	{
		std::string errorMsg = ( char* )errorBlob->GetBufferPointer();
		std::wstring wErrorMsg;
		wErrorMsg.assign( errorMsg.begin(), errorMsg.end() );
		AEMessageBox( wErrorMsg );
	}

	CHECK_HRESULT( device->CreateRootSignature( 0, rootBlob->GetBufferPointer(), rootBlob->GetBufferSize(), IID_PPV_ARGS( &rootSignature ) ),
				   L"Failed to create root signature" );

	return rootSignature;
}

ID3D12RootSignature* CreateComputeRootSignature( ID3D12Device* device, const GPIPipelineStateDesc& pipelineDesc )
{
	ID3D12RootSignature* rootSignature;

	D3D12_DESCRIPTOR_RANGE descRange{};
	descRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
	descRange.NumDescriptors = 1;
	descRange.BaseShaderRegister = 0;

	D3D12_ROOT_PARAMETER rootParam{};
	rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParam.DescriptorTable.NumDescriptorRanges = 1;
	rootParam.DescriptorTable.pDescriptorRanges = &descRange;

	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
	rootSignatureDesc.NumParameters = 1;
	rootSignatureDesc.pParameters = &rootParam;
	rootSignatureDesc.NumStaticSamplers = 0;
	rootSignatureDesc.pStaticSamplers = nullptr;
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	ID3DBlob* rootBlob;
	ID3DBlob* errorBlob;
	CHECK_HRESULT( D3D12SerializeRootSignature( &rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &rootBlob, &errorBlob ),
				   L"Failed to serialize root signature" );

	CHECK_HRESULT( device->CreateRootSignature( 0, rootBlob->GetBufferPointer(), rootBlob->GetBufferSize(), IID_PPV_ARGS( &rootSignature ) ),
				   L"Failed to create root signature" );

	return rootSignature;
}

ID3D12PipelineState* CreateGraphicsPipelineState( ID3D12Device* device, const GPIPipelineStateDesc& pipelineDesc, ID3D12RootSignature* rootSignature, ID3DBlob* vertexShader, ID3DBlob* pixelShader )
{
	ID3D12PipelineState* pso;

	std::vector<D3D12_INPUT_ELEMENT_DESC> layout;
	layout.resize( pipelineDesc.inputDesc.size() );
	for( uint32 index = 0; index < layout.size(); ++index )
	{
		layout[ index ] = TranslateInputDesc( pipelineDesc.inputDesc[ index ] );
	}

	D3D12_RASTERIZER_DESC rasterizerDesc{};
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	rasterizerDesc.FrontCounterClockwise = FALSE;
	rasterizerDesc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
	rasterizerDesc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
	rasterizerDesc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
	rasterizerDesc.DepthClipEnable = TRUE;
	rasterizerDesc.MultisampleEnable = FALSE;
	rasterizerDesc.AntialiasedLineEnable = FALSE;
	rasterizerDesc.ForcedSampleCount = 0;
	rasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	D3D12_BLEND_DESC blendState{};
	blendState.AlphaToCoverageEnable = FALSE;
	blendState.IndependentBlendEnable = FALSE;
	for( uint32 Index = 0; Index < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++Index )
	{
		const D3D12_RENDER_TARGET_BLEND_DESC defaultRenderTargetBlendDesc =
		{
			FALSE,
			FALSE,
			D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
			D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
			D3D12_LOGIC_OP_NOOP,
			D3D12_COLOR_WRITE_ENABLE_ALL,
		};
		blendState.RenderTarget[ Index ] = defaultRenderTargetBlendDesc;
	}
	// Simple alpha blending
	for( uint32 index = 0; index < pipelineDesc.numRenderTargets; ++index )
	{
		blendState.RenderTarget[ index ].BlendEnable = true;
		blendState.RenderTarget[ index ].SrcBlend = D3D12_BLEND_SRC_ALPHA;
		blendState.RenderTarget[ index ].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
		blendState.RenderTarget[ index ].BlendOp = D3D12_BLEND_OP_ADD;
		blendState.RenderTarget[ index ].SrcBlendAlpha = D3D12_BLEND_ONE;
		blendState.RenderTarget[ index ].DestBlendAlpha = D3D12_BLEND_ZERO;
		blendState.RenderTarget[ index ].BlendOpAlpha = D3D12_BLEND_OP_ADD;
		blendState.RenderTarget[ index ].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	}

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
	psoDesc.RasterizerState = rasterizerDesc;
	psoDesc.BlendState = blendState;
	psoDesc.VS.BytecodeLength = vertexShader->GetBufferSize();
	psoDesc.VS.pShaderBytecode = vertexShader->GetBufferPointer();
	psoDesc.PS.BytecodeLength = pixelShader->GetBufferSize();
	psoDesc.PS.pShaderBytecode = pixelShader->GetBufferPointer();
	psoDesc.pRootSignature = rootSignature;
	psoDesc.NumRenderTargets = pipelineDesc.numRenderTargets;
	for( uint32 index = 0; index < pipelineDesc.numRenderTargets; ++index )
	{
		psoDesc.RTVFormats[ index ] = TranslateBufferFormat( pipelineDesc.renderTargetDesc[ index ].format );
	}
	psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	psoDesc.InputLayout.NumElements = layout.size();
	psoDesc.InputLayout.pInputElementDescs = layout.data();
	psoDesc.SampleDesc.Count = 1;
	psoDesc.DepthStencilState.DepthEnable = pipelineDesc.bWriteDepth;
	psoDesc.DepthStencilState.DepthWriteMask = pipelineDesc.bWriteDepth ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
	psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	psoDesc.DepthStencilState.StencilEnable = false;
	psoDesc.SampleMask = 0xFFFFFFFF;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	device->CreateGraphicsPipelineState( &psoDesc, IID_PPV_ARGS( &pso ) );

	return pso;
}

ID3D12PipelineState* CreateComputePipelineState( ID3D12Device* device, ID3D12RootSignature* rootSignature, ID3DBlob* computeShader )
{
	ID3D12PipelineState* pso;

	D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc{};
	psoDesc.pRootSignature = rootSignature;
	psoDesc.CS.BytecodeLength = computeShader->GetBufferSize();
	psoDesc.CS.pShaderBytecode = computeShader->GetBufferPointer();

	device->CreateComputePipelineState( &psoDesc, IID_PPV_ARGS( &pso ) );

	return pso;
}

void GPI_DX12::CreatePipelineState( const GPIPipelineStateDesc& pipelineDesc )
{
	uint32 pipelineHash = pipelineDesc.hash;
	if( _pipelineCache.contains( pipelineHash ) )
	{
		return;
	}

	std::shared_ptr< GPIPipeline_DX12 > pipelineState( new GPIPipeline_DX12() );
	_pipelineCache.emplace( pipelineHash, pipelineState );

	if( pipelineDesc.pipelineType == PipelineType_Graphics )
	{
		ID3DBlob* vertexShader = CreateShader( pipelineDesc.vertexShader );
		ID3DBlob* pixelShader = CreateShader( pipelineDesc.pixelShader );

		assert( !_shaderCache.contains( pipelineDesc.vertexShader.hash ) );
		_shaderCache.emplace( pipelineDesc.vertexShader.hash, vertexShader );
		_shaderCache.emplace( pipelineDesc.pixelShader.hash, pixelShader );

		pipelineState->rootSignature = CreateGraphicsRootSignature( _device, pipelineDesc );
		pipelineState->pipelineState = CreateGraphicsPipelineState( _device, pipelineDesc, pipelineState->rootSignature, vertexShader, pixelShader );

		if( pipelineDesc.constBufferSize > 0 )
		{
			pipelineState->constBuffer = CreateConstantBuffer( nullptr, pipelineDesc.constBufferSize );
		}
		if( pipelineDesc.numResources > 0 )
		{
			pipelineState->resourceBuffers.resize( pipelineDesc.numResources );
		}
	}
	else if( pipelineDesc.pipelineType == PipelineType_Compute )
	{
		ID3DBlob* computeShader = CreateShader( pipelineDesc.computeShader );

		assert( !_shaderCache.contains( pipelineDesc.computeShader.hash ) );
		_shaderCache.emplace( pipelineDesc.computeShader.hash, computeShader );

		pipelineState->rootSignature = CreateComputeRootSignature( _device, pipelineDesc );
		pipelineState->pipelineState = CreateComputePipelineState( _device, pipelineState->rootSignature, computeShader );

		if( pipelineDesc.constBufferSize > 0 )
		{
			pipelineState->constBuffer = CreateConstantBuffer( nullptr, pipelineDesc.constBufferSize );
		}
	}
}

void GPI_DX12::BindResourceBuffer( const GPIPipelineStateDesc& pipelineDesc, IVertexBuffer* resourceBuffer, uint32 index )
{
	uint32 pipelineHash = GetPipelineStateHash( pipelineDesc );
	assert( _pipelineCache.contains( pipelineHash ) );

	std::shared_ptr<GPIPipeline_DX12>& pipelineState = _pipelineCache[ pipelineHash ];
	pipelineState->resourceBuffers[ index ] = resourceBuffer;
}

void GPI_DX12::UpdateConstantBuffer( uint32 bufferHash, void* data, uint32 size )
{
	UpdateBuffer( _device, _cmdQueueCtx[ D3D12_COMMAND_LIST_TYPE_COPY ], _constBuffer, data, size );
}

void GPI_DX12::UpdateConstantBuffer1( const GPIPipelineStateDesc& pipelineDesc, void* data )
{
	uint32 pipelineHash = GetPipelineStateHash( pipelineDesc );
	assert( _pipelineCache.contains( pipelineHash ) );

	std::shared_ptr<GPIPipeline_DX12>& pipelineState = _pipelineCache[ pipelineHash ];

	if( pipelineDesc.constBufferSize > 0 )
	{
		UpdateBuffer( _device, _cmdQueueCtx[ D3D12_COMMAND_LIST_TYPE_COPY ], pipelineState->constBuffer, data, pipelineDesc.constBufferSize );
	}
}

void GPI_DX12::RunCS()
{
	CHECK_HRESULT( _cmdQueueCtx[ D3D12_COMMAND_LIST_TYPE_COMPUTE ].allocator->Reset(), L"Failed to reset command allocator." );
	CHECK_HRESULT( ( *_cmdQueueCtx[ D3D12_COMMAND_LIST_TYPE_COMPUTE ].cmdListIter )->Reset( _cmdQueueCtx[ D3D12_COMMAND_LIST_TYPE_COMPUTE ].allocator, nullptr ), L"Failed to reset command list." );

	static ID3D12Resource* buffer = nullptr;
	if( !buffer )
	{
		_guavHeap = CreateDescriptorHeap( _device, 4, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE );

		int32 data[ 64 ] = {};
		buffer = CreateBuffer( _device, _cmdQueueCtx[ D3D12_COMMAND_LIST_TYPE_COPY ], data, 256 );

		TransitionResource( *_cmdQueueCtx[ D3D12_COMMAND_LIST_TYPE_COMPUTE ].cmdListIter, buffer, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS );

		D3D12_CPU_DESCRIPTOR_HANDLE uavDescHandle = _guavHeap->GetCPUDescriptorHandleForHeapStart();

		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
		uavDesc.Format = DXGI_FORMAT_R32_UINT;
		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
		uavDesc.Buffer.FirstElement = 0;
		uavDesc.Buffer.CounterOffsetInBytes = 0;
		uavDesc.Buffer.NumElements = 64;
		uavDesc.Buffer.StructureByteStride = 0;
		uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

		_device->CreateUnorderedAccessView( buffer, nullptr, &uavDesc, uavDescHandle );
	}

	static ID3D12RootSignature* rootSignature = nullptr;
	static ID3D12PipelineState* pipelineState = nullptr;
	if( !rootSignature )
	{
		// temp
		//rootSignature = CreateRootSignature1();
		//pipelineState = CreatePipelineState1( rootSignature );
	}

	CommandQueueContext& cmdQueueCtx = _cmdQueueCtx[ D3D12_COMMAND_LIST_TYPE_COMPUTE ];
	ID3D12GraphicsCommandList* cmdList = *cmdQueueCtx.cmdListIter;

	cmdList->SetPipelineState( pipelineState );
	cmdList->SetComputeRootSignature( rootSignature );

	cmdList->SetDescriptorHeaps( 1, &_guavHeap );
	cmdList->SetComputeRootDescriptorTable( 0, _guavHeap->GetGPUDescriptorHandleForHeapStart() );

	cmdList->Dispatch( 1, 1, 1 );

	CHECK_HRESULT( cmdList->Close(), L"Failed to close command list." );

	ID3D12Fence* fence = cmdQueueCtx.fence;
	HANDLE fenceEventHandle = cmdQueueCtx.fenceEventHandle;
	ID3D12CommandList* cmdListInterface = cmdList;
	cmdQueueCtx.cmdQueue->ExecuteCommandLists( 1, &cmdListInterface );

	WaitForFence( cmdQueueCtx );
}