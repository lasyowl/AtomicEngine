#include "stdafx.h"
#include "GPI_DX12.h"
#include "EngineDefine.h"
#include "AtomicEngine.h"
#include "DebugUtil.h"
#include "RawImage.h"
#include "AssetLoader.h"
#include "GPIShader_DX12.h"
#include "GPIPipeline_DX12.h"
#include "GPIUtility_DX12.h"

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

////////////////////////////////
// Misc Functions
////////////////////////////////
constexpr D3D12_VIEWPORT ToDX12Viewport( uint32 width, uint32 height )
{
	return D3D12_VIEWPORT{ 0, 0, ( float )width, ( float )height, 0, 1 };
}

constexpr D3D12_RECT ToDX12Rect( uint32 width, uint32 height )
{
	return D3D12_RECT{ 0, 0, ( int32 )width, ( int32 )height };
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

D3D12_INPUT_ELEMENT_DESC TranslateInputDesc( const GPIPipelineInputDesc& inputDesc )
{
	D3D12_INPUT_ELEMENT_DESC d3dInputDesc =
	{
		inputDesc.semanticName.c_str(),
		0,
		GPIUtil::TranslateResourceFormat( inputDesc.format ),
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

void CopyMemoryToBuffer( ID3D12Resource* buffer, void* data, uint64 size )
{
	void* virtualMem;
	buffer->Map( 0, nullptr, &virtualMem );
	::memcpy( virtualMem, data, size );
	buffer->Unmap( 0, nullptr );
}

void UpdateTexture( ID3D12Device* device, CommandQueueContext& cmdQueueCtx, ID3D12Resource* outBuffer, void* data, uint32 width, uint32 height )
{
	ID3D12GraphicsCommandList* cmdList = *cmdQueueCtx.iCmdList;
	ID3D12CommandAllocator* cmdAllocator = cmdQueueCtx.allocator;

	D3D12_RESOURCE_DESC resourceDesc = outBuffer->GetDesc();
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

	//TransitionResource( cmdList, outBuffer, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST );

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

	//TransitionResource( cmdList, outBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE );

	CHECK_HRESULT( cmdList->Close(), L"Failed to close command list." );

	ID3D12CommandList* cmdListInterface = cmdList;
	cmdQueueCtx.cmdQueue->ExecuteCommandLists( 1, &cmdListInterface );
	
	WaitForFence( cmdQueueCtx );

	uploadBuffer->Release();
}

////////////////////////////////
// GPIResourceAllocator_DX12
////////////////////////////////
void GPIResourceAllocator_DX12::Initialize()
{
	constexpr uint32 NUM_RESOURCES = 10000;
	for( uint32 iter = 0; iter < NUM_RESOURCES; ++iter )
	{
		_freeResourceIDs.push( iter );
	}
}

uint32 GPIResourceAllocator_DX12::AllocateUniqueResourceID( ID3D12Resource* resource )
{
	uint32 resourceID = _freeResourceIDs.front();
	_freeResourceIDs.pop();

	assert( !_resourceCache.contains( resourceID ) );
	_resourceCache[ resourceID ] = resource;

	return resourceID;
}

void GPIResourceAllocator_DX12::ReleaseUniqueResourceID( uint32 resourceID )
{
	_freeResourceIDs.push( resourceID );
}

uint32 GPIResourceAllocator_DX12::CreateResource( ID3D12Device* device, const GPIResourceDesc& desc )
{
	// todo : set clear value for depthstencil and others..

	D3D12_RESOURCE_DESC translatedDesc = GPIUtil::TranslateResourceDesc( desc );
	D3D12_HEAP_PROPERTIES defaultHeapProp = HeapProperties( D3D12_HEAP_TYPE_DEFAULT );

	ID3D12Resource* resource;
	CHECK_HRESULT( device->CreateCommittedResource( &defaultHeapProp,
				   D3D12_HEAP_FLAG_NONE,
				   &translatedDesc,
				   GPIUtil::TranslateResourceState( desc.initialState ),
				   nullptr,
				   IID_PPV_ARGS( &resource ) ),
				   L"Failed to create output buffer." );

	return AllocateUniqueResourceID( resource );
}

ID3D12Resource* GPIResourceAllocator_DX12::GetResource( uint32 resourceID )
{
	assert( _resourceCache.contains( resourceID ) );

	return _resourceCache[ resourceID ];
}

///////////////////////////////////////
// GPI_DX12
///////////////////////////////////////
GPI_DX12::GPI_DX12( const HWND hWnd, const int32 screenWidth, const int32 screenHeight )
	: _device( nullptr )
	, _swapChain( nullptr )
	, _swapChainIndex( 0 )
	, _debugInterface( nullptr )
	, _debugInfoQueue( nullptr )
	, _hWnd( hWnd )
{
	SetWindowSize( screenWidth, screenHeight );
}

void GPI_DX12::Initialize()
{
	/* Cretate and enable debug layer */
	CHECK_HRESULT( D3D12GetDebugInterface( IID_PPV_ARGS( &_debugInterface ) ), L"Failed to create debug layer." );
	_debugInterface->EnableDebugLayer();

	CHECK_HRESULT( D3D12GetDebugInterface( IID_PPV_ARGS( &_debugInterfaceEx ) ), L"Failed to create debug layer." );
	_debugInterfaceEx->SetAutoBreadcrumbsEnablement( D3D12_DRED_ENABLEMENT_FORCED_ON );
	_debugInterfaceEx->SetPageFaultEnablement( D3D12_DRED_ENABLEMENT_FORCED_ON );

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
			cmdQueueCtx.iCmdList = cmdQueueCtx.cmdLists.begin();
		}
	}

	/* Create swapchain */
	{
		CommandQueueContext& cmdQueueCtx = _cmdQueueCtx[ D3D12_COMMAND_LIST_TYPE_DIRECT ];

		DXGI_SWAP_CHAIN_DESC swapChainDesc{};
		swapChainDesc.BufferCount = SWAPCHAIN_COUNT;
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferDesc.Width = _windowWidth;
		swapChainDesc.BufferDesc.Height = _windowHeight;
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

	_heap.Initialize( _device );
	_resource.Initialize();
}

void GPI_DX12::BeginFrame( const IGPIResource& inSwapChainResource, const IGPIRenderTargetView& inSwapChainRTV, const IGPIDepthStencilView& inSwapChainDSV )
{
	const GPIResource_DX12& swapChainResource = static_cast< const GPIResource_DX12& >( inSwapChainResource );
	const GPIRenderTargetView_DX12& swapChainRTV = static_cast< const GPIRenderTargetView_DX12& >( inSwapChainRTV );
	const GPIDepthStencilView_DX12& swapChainDSV = static_cast< const GPIDepthStencilView_DX12& >( inSwapChainDSV );

	CommandQueueContext& cmdQueueCtx = _cmdQueueCtx[ D3D12_COMMAND_LIST_TYPE_DIRECT ];
	ID3D12CommandAllocator* cmdAllocator = cmdQueueCtx.allocator;
	ID3D12GraphicsCommandList* cmdList = *cmdQueueCtx.iCmdList;

	CHECK_HRESULT( cmdAllocator->Reset(), L"Failed to reset command allocator." );
	CHECK_HRESULT( cmdList->Reset( cmdAllocator, nullptr ), L"Failed to reset command list." );

	TransitionResource( cmdList, swapChainResource.resource, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET );

	static float clearColor[] = { 0.242f, 0.242f, 0.442f, 1 };

	cmdList->ClearRenderTargetView( swapChainRTV.handle.cpu, clearColor, 0, nullptr );
	cmdList->ClearDepthStencilView( swapChainDSV.handle.cpu, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr );

	CHECK_HRESULT( cmdList->Close(), L"Failed to close command list." );

	ID3D12CommandList* cmdListInterface = cmdList;
	cmdQueueCtx.cmdQueue->ExecuteCommandLists( 1, &cmdListInterface );

	WaitForFence( cmdQueueCtx );
}

void GPI_DX12::EndFrame( const IGPIResource& inSwapChainResource )
{
	const GPIResource_DX12& swapChainResource = static_cast< const GPIResource_DX12& >( inSwapChainResource );

	CommandQueueContext& cmdQueueCtx = _cmdQueueCtx[ D3D12_COMMAND_LIST_TYPE_DIRECT ];
	ID3D12CommandAllocator* cmdAllocator = cmdQueueCtx.allocator;
	ID3D12GraphicsCommandList* cmdList = *cmdQueueCtx.iCmdList;

	CHECK_HRESULT( cmdAllocator->Reset(), L"Failed to reset command allocator." );
	CHECK_HRESULT( cmdList->Reset( cmdAllocator, nullptr ), L"Failed to reset command list." );

	TransitionResource( cmdList, swapChainResource.resource, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT );

	CHECK_HRESULT( cmdList->Close(), L"Failed to close command list." );
	
	ID3D12CommandList* cmdListInterface = cmdList;
	cmdQueueCtx.cmdQueue->ExecuteCommandLists( 1, &cmdListInterface );

	CHECK_HRESULT( _swapChain->Present( 1, 0 ), L"Failed to present swapchain." );

	WaitForFence( cmdQueueCtx );

	_swapChainIndex = ( _swapChainIndex + 1 ) % SWAPCHAIN_COUNT;

	if( ++cmdQueueCtx.iCmdList == cmdQueueCtx.cmdLists.end() )
	{
		cmdQueueCtx.iCmdList = cmdQueueCtx.cmdLists.begin();
	}
}

void GPI_DX12::ClearSwapChain( const IGPIRenderTargetView& inRTV )
{
	const GPIRenderTargetView_DX12& rtv = static_cast< const GPIRenderTargetView_DX12& >( inRTV );

	CommandQueueContext& cmdQueueCtx = _cmdQueueCtx[ D3D12_COMMAND_LIST_TYPE_DIRECT ];
	ID3D12CommandAllocator* cmdAllocator = cmdQueueCtx.allocator;
	ID3D12GraphicsCommandList* cmdList = *cmdQueueCtx.iCmdList;

	CHECK_HRESULT( cmdAllocator->Reset(), L"Failed to reset command allocator." );
	CHECK_HRESULT( cmdList->Reset( cmdAllocator, nullptr ), L"Failed to reset command list." );

	TransitionResource( cmdList, rtv.resource, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_UNORDERED_ACCESS );

	uint32 clearValue[ 4 ]{};
	cmdList->ClearUnorderedAccessViewUint( rtv.handle.gpu, rtv.handle.cpu, rtv.resource, clearValue, 0, nullptr );

	TransitionResource( cmdList, rtv.resource, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_RENDER_TARGET );

	CHECK_HRESULT( cmdList->Close(), L"Failed to close command list." );

	ID3D12CommandList* cmdListInterface = cmdList;
	cmdQueueCtx.cmdQueue->ExecuteCommandLists( 1, &cmdListInterface );

	WaitForFence( cmdQueueCtx );
}

void GPI_DX12::ClearRenderTarget( const IGPIRenderTargetView& inRTV )
{
	const GPIRenderTargetView_DX12& rtv = static_cast< const GPIRenderTargetView_DX12& >( inRTV );

	CommandQueueContext& cmdQueueCtx = _cmdQueueCtx[ D3D12_COMMAND_LIST_TYPE_DIRECT ];
	ID3D12CommandAllocator* cmdAllocator = cmdQueueCtx.allocator;
	ID3D12GraphicsCommandList* cmdList = *cmdQueueCtx.iCmdList;

	CHECK_HRESULT( cmdAllocator->Reset(), L"Failed to reset command allocator." );
	CHECK_HRESULT( cmdList->Reset( cmdAllocator, nullptr ), L"Failed to reset command list." );

	TransitionResource( cmdList, rtv.resource, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_UNORDERED_ACCESS );

	uint32 clearValue[ 4 ]{};
	cmdList->ClearUnorderedAccessViewUint( rtv.handle.gpu, rtv.handle.cpu, rtv.resource, clearValue, 0, nullptr );

	TransitionResource( cmdList, rtv.resource, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_RENDER_TARGET );

	CHECK_HRESULT( cmdList->Close(), L"Failed to close command list." );

	ID3D12CommandList* cmdListInterface = cmdList;
	cmdQueueCtx.cmdQueue->ExecuteCommandLists( 1, &cmdListInterface );

	WaitForFence( cmdQueueCtx );
}

void GPI_DX12::ClearRenderTarget( const IGPIUnorderedAccessView& inUAV )
{
	const GPIUnorderedAccessView_DX12& uav = static_cast< const GPIUnorderedAccessView_DX12& >( inUAV );

	CommandQueueContext& cmdQueueCtx = _cmdQueueCtx[ D3D12_COMMAND_LIST_TYPE_DIRECT ];
	ID3D12CommandAllocator* cmdAllocator = cmdQueueCtx.allocator;
	ID3D12GraphicsCommandList* cmdList = *cmdQueueCtx.iCmdList;

	CHECK_HRESULT( cmdAllocator->Reset(), L"Failed to reset command allocator." );
	CHECK_HRESULT( cmdList->Reset( cmdAllocator, nullptr ), L"Failed to reset command list." );

	TransitionResource( cmdList, uav.resource, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_UNORDERED_ACCESS );

	uint32 clearValue[ 4 ]{};
	cmdList->ClearUnorderedAccessViewUint( uav.handle.gpu, uav.handle.cpu, uav.resource, clearValue, 0, nullptr );

	TransitionResource( cmdList, uav.resource, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_RENDER_TARGET );

	CHECK_HRESULT( cmdList->Close(), L"Failed to close command list." );

	ID3D12CommandList* cmdListInterface = cmdList;
	cmdQueueCtx.cmdQueue->ExecuteCommandLists( 1, &cmdListInterface );

	WaitForFence( cmdQueueCtx );
}

void GPI_DX12::SetPipelineState( const GPIPipelineStateDesc& desc, const GPIPipeline_DX12& pipeline )
{
	CommandQueueContext& cmdQueueCtx = _cmdQueueCtx[ D3D12_COMMAND_LIST_TYPE_DIRECT ];
	ID3D12CommandAllocator* cmdAllocator = cmdQueueCtx.allocator;
	ID3D12GraphicsCommandList* cmdList = *cmdQueueCtx.iCmdList;

	CHECK_HRESULT( cmdAllocator->Reset(), L"Failed to reset allocator." );
	CHECK_HRESULT( cmdList->Reset( cmdAllocator, nullptr ), L"Failed to reset command list." );

	cmdList->SetPipelineState( pipeline.pipelineState );
	cmdList->SetGraphicsRootSignature( pipeline.rootSignature );
	cmdList->IASetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

	if( desc.bBindDepth )
		cmdList->OMSetRenderTargets( pipeline.rtv.size(), pipeline.rtv.data(), true, &pipeline.dsv );
	else
		cmdList->OMSetRenderTargets( pipeline.rtv.size(), pipeline.rtv.data(), false, nullptr );

	D3D12_VIEWPORT viewport = ToDX12Viewport( _windowWidth, _windowHeight );
	D3D12_RECT scissorRect = ToDX12Rect( _windowWidth, _windowHeight );
	cmdList->RSSetViewports( 1, &viewport );
	cmdList->RSSetScissorRects( 1, &scissorRect );

	uint32 rootParamIndex = 0;
	for( uint32 index = 0; index < pipeline.cbv.size(); ++index )
	{
		const D3D12_GPU_VIRTUAL_ADDRESS& gpuAddress = pipeline.cbv[ index ];
		cmdList->SetGraphicsRootConstantBufferView( rootParamIndex++, gpuAddress );
	}
	for( uint32 index = 0; index < pipeline.srv.size(); ++index )
	{
		const D3D12_GPU_VIRTUAL_ADDRESS& gpuAddress = pipeline.srv[ index ];
		cmdList->SetGraphicsRootShaderResourceView( rootParamIndex++, gpuAddress );
	}
	for( uint32 index = 0; index < pipeline.uav.size(); ++index )
	{
		const D3D12_GPU_VIRTUAL_ADDRESS& gpuAddress = pipeline.uav[ index ];
		cmdList->SetGraphicsRootUnorderedAccessView( rootParamIndex++, gpuAddress );
	}

	ID3D12DescriptorHeap* textureHeap = _heap.GetHeap( GPIResourceViewType_SRV_TEXTURE );
	cmdList->SetDescriptorHeaps( 1, &textureHeap );

	for( uint32 index = 0; index < pipeline.textureTables.size(); ++index )
	{
		cmdList->SetGraphicsRootDescriptorTable( rootParamIndex++, pipeline.textureTables[ index ] );
	}
}

void GPI_DX12::SetPipelineState( const GPIPipelineStateDesc& pipelineDesc )
{
	assert( _pipelineCache.contains( pipelineDesc.id ) );

	std::shared_ptr<GPIPipeline_DX12>& pipeline = _pipelineCache[ pipelineDesc.id ];

	SetPipelineState( pipelineDesc, *pipeline.get() );
}

void GPI_DX12::Render( const GPIPipelineInput& pipelineInput )
{
	CommandQueueContext& cmdQueueCtx = _cmdQueueCtx[ D3D12_COMMAND_LIST_TYPE_DIRECT ];
	ID3D12GraphicsCommandList* cmdList = *cmdQueueCtx.iCmdList;

	std::vector<D3D12_VERTEX_BUFFER_VIEW> VBViews;
	VBViews.resize( pipelineInput.vbv.size() );
	for( uint32 index = 0; index < VBViews.size(); ++index )
	{
		D3D12_VERTEX_BUFFER_VIEW& vbv = VBViews[ index ];

		if( pipelineInput.vbv[ index ] )
		{
			const GPIVertexBufferView_DX12& inVBV = static_cast< const GPIVertexBufferView_DX12& >( *pipelineInput.vbv[ index ] );
			vbv.BufferLocation = inVBV.gpuAddress;
			vbv.SizeInBytes = inVBV.size;
			vbv.StrideInBytes = inVBV.stride;
		}
		else
		{
			vbv = {};
		}
	}

	for( const IGPIIndexBufferViewRef& ibv : pipelineInput.ibv )
	{
		const GPIIndexBufferView_DX12& inIBV = static_cast< const GPIIndexBufferView_DX12& >( *ibv );

		D3D12_INDEX_BUFFER_VIEW IBView{};
		IBView.BufferLocation = inIBV.gpuAddress;
		IBView.SizeInBytes = inIBV.size;
		IBView.Format = DXGI_FORMAT_R32_UINT;

		cmdList->IASetVertexBuffers( 0, VBViews.size(), VBViews.data());
		cmdList->IASetIndexBuffer( &IBView );
		cmdList->DrawIndexedInstanced( inIBV.size / sizeof( uint32 ), 1, 0, 0, 0 );
	}
}

void GPI_DX12::ExecuteCommandList()
{
	CommandQueueContext& cmdQueueCtx = _cmdQueueCtx[ D3D12_COMMAND_LIST_TYPE_DIRECT ];
	ID3D12GraphicsCommandList* cmdList = *cmdQueueCtx.iCmdList;

	CHECK_HRESULT( cmdList->Close(), L"Failed to close command list." );

	ID3D12CommandList* cmdListInterface = cmdList;
	cmdQueueCtx.cmdQueue->ExecuteCommandLists( 1, &cmdListInterface );

	WaitForFence( cmdQueueCtx );
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

	std::vector<D3D12_DESCRIPTOR_RANGE> textureRanges;
	textureRanges.resize( pipelineDesc.numTextures.size() );

	std::vector<D3D12_ROOT_PARAMETER> rootParams;
	rootParams.resize( pipelineDesc.numCBVs + pipelineDesc.numSRVs + pipelineDesc.numUAVs + pipelineDesc.numTextures.size() );
	{
		uint32 rootParamIndex = 0;
		D3D12_ROOT_DESCRIPTOR rootDesc{};

		for( uint32 index = 0; index < pipelineDesc.numCBVs; ++index, ++rootParamIndex )
		{
			rootDesc.ShaderRegister = index;

			rootParams[ rootParamIndex ].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
			rootParams[ rootParamIndex ].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
			rootParams[ rootParamIndex ].Descriptor = rootDesc;
		}

		for( uint32 index = 0; index < pipelineDesc.numSRVs; ++index, ++rootParamIndex )
		{
			rootDesc.ShaderRegister = index;

			rootParams[ rootParamIndex ].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
			rootParams[ rootParamIndex ].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
			rootParams[ rootParamIndex ].Descriptor = rootDesc;
		}

		for( uint32 index = 0; index < pipelineDesc.numUAVs; ++index, ++rootParamIndex )
		{
			rootDesc.ShaderRegister = index;

			rootParams[ rootParamIndex ].ParameterType = D3D12_ROOT_PARAMETER_TYPE_UAV;
			rootParams[ rootParamIndex ].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
			rootParams[ rootParamIndex ].Descriptor = rootDesc;
		}

		uint32 textureRegister = 0;
		for( uint32 index = 0; index < pipelineDesc.numTextures.size(); ++index, ++rootParamIndex )
		{
			D3D12_DESCRIPTOR_RANGE& range = textureRanges[ index ];
			range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
			range.NumDescriptors = pipelineDesc.numTextures[ index ];
			range.BaseShaderRegister = textureRegister;

			rootParams[ rootParamIndex ].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			rootParams[ rootParamIndex ].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
			rootParams[ rootParamIndex ].DescriptorTable.NumDescriptorRanges = 1;
			rootParams[ rootParamIndex ].DescriptorTable.pDescriptorRanges = &range;

			textureRegister += range.NumDescriptors;
		}
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
	for( uint32 index = 0; index < pipelineDesc.rtvFormats.size(); ++index )
	{
		blendState.RenderTarget[ index ].BlendEnable = pipelineDesc.bEnableBlend;
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
	psoDesc.NumRenderTargets = pipelineDesc.rtvFormats.size();
	for( uint32 index = 0; index < pipelineDesc.rtvFormats.size(); ++index )
	{
		psoDesc.RTVFormats[ index ] = GPIUtil::TranslateResourceFormat( pipelineDesc.rtvFormats[ index ] );
	}
	psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	psoDesc.InputLayout.NumElements = layout.size();
	psoDesc.InputLayout.pInputElementDescs = layout.data();
	psoDesc.SampleDesc.Count = 1;
	psoDesc.DepthStencilState.DepthEnable = pipelineDesc.bBindDepth;
	psoDesc.DepthStencilState.DepthWriteMask = pipelineDesc.bBindDepth ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
	psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	psoDesc.DepthStencilState.StencilEnable = false;
	psoDesc.SampleMask = 0xFFFFFFFF;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	device->CreateGraphicsPipelineState( &psoDesc, IID_PPV_ARGS( &pso ) );

	return pso;
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

uint32 GPI_DX12::GetSwapChainCurrentIndex()
{
	return _swapChainIndex;
}

IGPIResourceRef GPI_DX12::GetSwapChainResource( const uint32 index )
{
	ID3D12Resource* swapChainResource;
	CHECK_HRESULT( _swapChain->GetBuffer( index, IID_PPV_ARGS( &swapChainResource ) ), L"Failed to get swapchain buffer." );
	
	return std::make_shared<GPIResource_DX12>( swapChainResource );
}

IGPIPipelineRef GPI_DX12::CreatePipelineState( const GPIPipelineStateDesc& pipelineDesc )
{
	if( _pipelineCache.contains( pipelineDesc.id ) )
	{
		return nullptr;
	}

	std::shared_ptr< GPIPipeline_DX12 > pipelineState( new GPIPipeline_DX12() );
	_pipelineCache.emplace( pipelineDesc.id, pipelineState );

	if( pipelineDesc.pipelineType == PipelineType_Graphics )
	{
		ID3DBlob* vertexShader = CreateShader( pipelineDesc.vertexShader );
		ID3DBlob* pixelShader = CreateShader( pipelineDesc.pixelShader );

		/*assert( !_shaderCache.contains( pipelineDesc.vertexShader.hash ) );
		_shaderCache.emplace( pipelineDesc.vertexShader.hash, vertexShader );
		_shaderCache.emplace( pipelineDesc.pixelShader.hash, pixelShader );*/  

		pipelineState->rootSignature = CreateGraphicsRootSignature( _device, pipelineDesc );
		pipelineState->pipelineState = CreateGraphicsPipelineState( _device, pipelineDesc, pipelineState->rootSignature, vertexShader, pixelShader );

		pipelineState->rtv.resize( pipelineDesc.rtvFormats.size() );
		pipelineState->cbv.resize( pipelineDesc.numCBVs );
		pipelineState->srv.resize( pipelineDesc.numSRVs );
		pipelineState->uav.resize( pipelineDesc.numSRVs );
		pipelineState->textureTables.resize( pipelineDesc.numTextures.size() );
	}
	/*else if( pipelineDesc.pipelineType == PipelineType_Compute )
	{
		ID3DBlob* computeShader = CreateShader( pipelineDesc.computeShader );

		assert( !_shaderCache.contains( pipelineDesc.computeShader.hash ) );
		_shaderCache.emplace( pipelineDesc.computeShader.hash, computeShader );

		pipelineState->rootSignature = CreateComputeRootSignature( _device, pipelineDesc );
		pipelineState->pipelineState = CreateComputePipelineState( _device, pipelineState->rootSignature, computeShader );

		if( pipelineDesc.numCBVs > 0 )
		{
			pipelineState->constBuffers.resize( pipelineDesc.numCBVs );
		}
	}*/

	return pipelineState;
}

IGPIResourceRef GPI_DX12::CreateResource( const GPIResourceDesc& desc )
{
	// todo : set clear value for depthstencil and others..

	D3D12_RESOURCE_DESC translatedDesc = GPIUtil::TranslateResourceDesc( desc );
	D3D12_HEAP_PROPERTIES defaultHeapProp = HeapProperties( D3D12_HEAP_TYPE_DEFAULT );

	ID3D12Resource* resource;

	if( desc.clearValue.type == EGPIResourceClearValueType::None )
	{
		CHECK_HRESULT( _device->CreateCommittedResource( &defaultHeapProp,
					   D3D12_HEAP_FLAG_NONE,
					   &translatedDesc,
					   GPIUtil::TranslateResourceState( desc.initialState ),
					   nullptr,
					   IID_PPV_ARGS( &resource ) ),
					   L"Failed to create output buffer." );
	}
	else
	{
		D3D12_CLEAR_VALUE clearValue = GPIUtil::TranslateResourceClearValue( desc );

		CHECK_HRESULT( _device->CreateCommittedResource( &defaultHeapProp,
					   D3D12_HEAP_FLAG_NONE,
					   &translatedDesc,
					   GPIUtil::TranslateResourceState( desc.initialState ),
					   &clearValue,
					   IID_PPV_ARGS( &resource ) ),
					   L"Failed to create output buffer." );
	}

	if( !desc.name.empty() )
	{
		resource->SetName( desc.name.c_str() );
	}

	return std::make_shared<GPIResource_DX12>( resource );
}

IGPIResourceRef GPI_DX12::CreateResource( const GPIResourceDesc& desc, void* data, uint32 sizeInBytes )
{
	CommandQueueContext& cmdQueueCtx = _cmdQueueCtx[ D3D12_COMMAND_LIST_TYPE_DIRECT ];
	ID3D12GraphicsCommandList* cmdList = *cmdQueueCtx.iCmdList;
	ID3D12CommandAllocator* cmdAllocator = cmdQueueCtx.allocator;

	D3D12_RESOURCE_DESC translatedDesc = GPIUtil::TranslateResourceDesc( desc );
	D3D12_HEAP_PROPERTIES defaultHeapProp = HeapProperties( D3D12_HEAP_TYPE_DEFAULT );
	D3D12_HEAP_PROPERTIES uploadHeapProp = HeapProperties( D3D12_HEAP_TYPE_UPLOAD );

	ID3D12Resource* resource;
	CHECK_HRESULT( _device->CreateCommittedResource( &defaultHeapProp,
				   D3D12_HEAP_FLAG_NONE,
				   &translatedDesc,
				   D3D12_RESOURCE_STATE_COPY_DEST,
				   nullptr,
				   IID_PPV_ARGS( &resource ) ),
				   L"Failed to create output buffer." );

	if( !desc.name.empty() )
	{
		resource->SetName( desc.name.c_str() );
	}

	// Create upload buffer on CPU
	ID3D12Resource* uploadResource;
	CHECK_HRESULT( _device->CreateCommittedResource( &uploadHeapProp,
				   D3D12_HEAP_FLAG_NONE,
				   &translatedDesc,
				   D3D12_RESOURCE_STATE_GENERIC_READ,
				   nullptr,
				   IID_PPV_ARGS( &uploadResource ) ),
				   L"Failed to create upload buffer." );

	if( data )
	{
		CopyMemoryToBuffer( uploadResource, data, sizeInBytes );
	}

	CHECK_HRESULT( cmdAllocator->Reset(), L"Failed to reset command allocator." );
	CHECK_HRESULT( cmdList->Reset( cmdAllocator, nullptr ), L"Failed to reset command list." );

	cmdList->CopyBufferRegion( resource, 0, uploadResource, 0, sizeInBytes );

	TransitionResource( cmdList, resource, D3D12_RESOURCE_STATE_COPY_DEST, GPIUtil::TranslateResourceState( desc.initialState ) );

	CHECK_HRESULT( cmdList->Close(), L"Failed to close command list." );

	ID3D12CommandList* cmdListInterface = cmdList;
	cmdQueueCtx.cmdQueue->ExecuteCommandLists( 1, &cmdListInterface );

	WaitForFence( cmdQueueCtx );

	uploadResource->Release();

	return std::make_shared<GPIResource_DX12>( resource );
}

IGPIRenderTargetViewRef GPI_DX12::CreateRenderTargetView( const IGPIResource& inResource, const GPIRenderTargetViewDesc& rtvDesc )
{
	const GPIResource_DX12& resource = static_cast< const GPIResource_DX12& >( inResource );

	std::shared_ptr<GPIRenderTargetView_DX12> rtv = std::make_shared<GPIRenderTargetView_DX12>();
	rtv->handle = _heap.Allocate( GPIResourceViewType_RTV );
	rtv->resource = resource.resource;

	D3D12_RENDER_TARGET_VIEW_DESC translatedDesc = GPIUtil::TranslateRTVDesc( rtvDesc );
	_device->CreateRenderTargetView( resource.resource, &translatedDesc, rtv->handle.cpu );

	return rtv;
}

IGPIDepthStencilViewRef GPI_DX12::CreateDepthStencilView( const IGPIResource& inResource, const GPIDepthStencilViewDesc& dsvDesc )
{
	const GPIResource_DX12& resource = static_cast< const GPIResource_DX12& >( inResource );

	std::shared_ptr<GPIDepthStencilView_DX12> dsv = std::make_shared<GPIDepthStencilView_DX12>();
	dsv->handle = _heap.Allocate( GPIResourceViewType_DSV );
	dsv->resource = resource.resource;

	D3D12_DEPTH_STENCIL_VIEW_DESC translatedDesc = GPIUtil::TranslateDSVDesc( dsvDesc );
	_device->CreateDepthStencilView( resource.resource, &translatedDesc, dsv->handle.cpu );

	return dsv;
}

IGPIConstantBufferViewRef GPI_DX12::CreateConstantBufferView( const IGPIResource& inResource, const GPIConstantBufferViewDesc& cbvDesc )
{
	const GPIResource_DX12& resource = static_cast< const GPIResource_DX12& >( inResource );

	//@TODO: refactoring needed
	const D3D12_RESOURCE_DESC desc = resource.resource->GetDesc();
	const bool bUseVirtualAddress = desc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER;
	//

	std::shared_ptr<GPIConstantBufferView_DX12> cbv = std::make_shared<GPIConstantBufferView_DX12>();
	cbv->handle = _heap.Allocate( GPIResourceViewType_CBV );
	cbv->resource = resource.resource;
	cbv->gpuAddress = bUseVirtualAddress ? resource.resource->GetGPUVirtualAddress() : 0;

	D3D12_CONSTANT_BUFFER_VIEW_DESC translatedDesc = GPIUtil::TranslateCBVDesc( inResource, cbvDesc );
	_device->CreateConstantBufferView( &translatedDesc, cbv->handle.cpu );

	return cbv;
}

IGPIShaderResourceViewRef GPI_DX12::CreateShaderResourceView( const IGPIResource& inResource, const GPIShaderResourceViewDesc& srvDesc )
{
	const GPIResource_DX12& resource = static_cast< const GPIResource_DX12& >( inResource );

	//@TODO: refactoring needed
	const D3D12_RESOURCE_DESC desc = resource.resource->GetDesc();
	const bool bUseVirtualAddress = desc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER;
	//

	std::shared_ptr<GPIShaderResourceView_DX12> srv = std::make_shared<GPIShaderResourceView_DX12>();
	srv->handle = _heap.Allocate( GPIResourceViewType_SRV );
	srv->resource = resource.resource;
	srv->gpuAddress = bUseVirtualAddress ? resource.resource->GetGPUVirtualAddress() : 0;

	D3D12_SHADER_RESOURCE_VIEW_DESC translatedDesc = GPIUtil::TranslateSRVDesc( inResource, srvDesc );
	_device->CreateShaderResourceView( resource.resource, &translatedDesc, srv->handle.cpu );

	return srv;
}

IGPIUnorderedAccessViewRef GPI_DX12::CreateUnorderedAccessView( const IGPIResource& inResource, const GPIUnorderedAccessViewDesc& uavDesc, const bool bShaderHidden )
{
	const GPIResource_DX12& resource = static_cast< const GPIResource_DX12& >( inResource );

	//@TODO: refactoring needed
	const D3D12_RESOURCE_DESC desc = resource.resource->GetDesc();
	const bool bUseVirtualAddress = desc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER;
	//

	std::shared_ptr<GPIUnorderedAccessView_DX12> uav = std::make_shared<GPIUnorderedAccessView_DX12>();
	uav->handle = _heap.Allocate( bShaderHidden ? GPIResourceViewType_UAV_SHADERHIDDEN : GPIResourceViewType_UAV );
	uav->resource = resource.resource;
	uav->gpuAddress = bUseVirtualAddress ? resource.resource->GetGPUVirtualAddress() : 0;

	D3D12_UNORDERED_ACCESS_VIEW_DESC translatedDesc = GPIUtil::TranslateUAVDesc( inResource, uavDesc );
	_device->CreateUnorderedAccessView( resource.resource, nullptr, &translatedDesc, uav->handle.cpu );

	return uav;
}

IGPITextureViewTableRef GPI_DX12::CreateTextureViewTable( const std::vector<const IGPIResource*> inResources, const std::vector<GPIShaderResourceViewDesc> inDescs )
{
	std::vector<GPIDescriptorHeapHandle_DX12> handles;
	handles.resize( inResources.size() );

	for( uint32 index = 0; index < handles.size(); ++index )
	{
		const GPIResource_DX12& resource = static_cast< const GPIResource_DX12& >( *inResources[ index ] );
		D3D12_SHADER_RESOURCE_VIEW_DESC translatedDesc = GPIUtil::TranslateSRVDesc( resource, inDescs[ index ] );

		handles[ index ] = _heap.Allocate( GPIResourceViewType_SRV_TEXTURE );

		if( inDescs[ index ].format == EGPIResourceFormat::R32_Float )
		{
			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
			srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvDesc.Texture2D.MipLevels = 1;

			_device->CreateShaderResourceView( resource.resource, &srvDesc, handles[ index ].cpu );

			continue;
		}

		_device->CreateShaderResourceView( resource.resource, &translatedDesc, handles[ index ].cpu );
	}

	std::shared_ptr<GPITextureViewTable_DX12> table = std::make_shared<GPITextureViewTable_DX12>();
	table->handle = handles[ 0 ];

//#pragma warning "[Danger] There are possibilities of sparse memory allocation!"
	
	return table;
}

IGPISamplerRef GPI_DX12::CreateSampler( const IGPIResource& inResource, const GPISamplerDesc& samplerDesc )
{
	return nullptr;
}

IGPIVertexBufferViewRef GPI_DX12::CreateVertexBufferView( const IGPIResource& inResource, const uint32 size, const uint32 stride )
{
	const GPIResource_DX12& resource = static_cast< const GPIResource_DX12& >( inResource );

	std::shared_ptr<GPIVertexBufferView_DX12> vb = std::make_shared<GPIVertexBufferView_DX12>();
	vb->gpuAddress = resource.resource->GetGPUVirtualAddress();
	vb->size = size;
	vb->stride = stride;

	return vb;
}

IGPIIndexBufferViewRef GPI_DX12::CreateIndexBufferView( const IGPIResource& inResource, const uint32 size )
{
	const GPIResource_DX12& resource = static_cast< const GPIResource_DX12& >( inResource );

	std::shared_ptr<GPIIndexBufferView_DX12> ib = std::make_shared<GPIIndexBufferView_DX12>();
	ib->gpuAddress = resource.resource->GetGPUVirtualAddress();
	ib->size = size;

	return ib;
}

void GPI_DX12::BindRenderTargetView( IGPIPipeline& inPipeline, const IGPIRenderTargetView& inRTV, uint32 index )
{
	const GPIRenderTargetView_DX12& rtv = static_cast< const GPIRenderTargetView_DX12& >( inRTV );

	GPIPipeline_DX12& pipeline = static_cast< GPIPipeline_DX12& >( inPipeline );
	pipeline.rtv[ index ] = rtv.handle.cpu;
}

void GPI_DX12::BindConstantBufferView( IGPIPipeline& inPipeline, const IGPIConstantBufferView& inCBV, uint32 index )
{
	const GPIConstantBufferView_DX12& cbv = static_cast< const GPIConstantBufferView_DX12& >( inCBV );

	GPIPipeline_DX12& pipeline = static_cast< GPIPipeline_DX12& >( inPipeline );
	pipeline.cbv[ index ] = cbv.gpuAddress;
}

void GPI_DX12::BindShaderResourceView( IGPIPipeline& inPipeline, const IGPIShaderResourceView& inSRV, uint32 index )
{
	const GPIShaderResourceView_DX12& srv = static_cast< const GPIShaderResourceView_DX12& >( inSRV );

	GPIPipeline_DX12& pipeline = static_cast< GPIPipeline_DX12& >( inPipeline );
	pipeline.srv[ index ] = srv.gpuAddress;
}

void GPI_DX12::BindUnorderedAccessView( IGPIPipeline& inPipeline, const IGPIUnorderedAccessView& inUAV, uint32 index )
{
	const GPIUnorderedAccessView_DX12& uav = static_cast< const GPIUnorderedAccessView_DX12& >( inUAV );

	GPIPipeline_DX12& pipeline = static_cast< GPIPipeline_DX12& >( inPipeline );
	pipeline.uav[ index ] = uav.gpuAddress;
}

void GPI_DX12::BindDepthStencilView( IGPIPipeline& inPipeline, const IGPIDepthStencilView& inDSV )
{
	const GPIDepthStencilView_DX12& dsv = static_cast< const GPIDepthStencilView_DX12& >( inDSV );

	GPIPipeline_DX12& pipeline = static_cast< GPIPipeline_DX12& >( inPipeline );
	pipeline.dsv = dsv.handle.cpu;
}

void GPI_DX12::BindTextureViewTable( IGPIPipeline& inPipeline, const IGPITextureViewTable& inTable, const uint32 index )
{
	const GPITextureViewTable_DX12& table = static_cast< const GPITextureViewTable_DX12& >( inTable );

	GPIPipeline_DX12& pipeline = static_cast< GPIPipeline_DX12& >( inPipeline );
	pipeline.textureTables[ index ] = table.handle.gpu;
}

void GPI_DX12::UpdateResourceData( const IGPIResource& inResource, void* data, uint32 sizeInBytes )
{
	CommandQueueContext& cmdQueueCtx = _cmdQueueCtx[ D3D12_COMMAND_LIST_TYPE_DIRECT ];

	ID3D12GraphicsCommandList* cmdList = *cmdQueueCtx.iCmdList;
	ID3D12CommandAllocator* cmdAllocator = cmdQueueCtx.allocator;

	const GPIResource_DX12& resource = static_cast< const GPIResource_DX12& >( inResource );

	D3D12_RESOURCE_DESC uploadBufferDesc = resource.resource->GetDesc();
	D3D12_HEAP_PROPERTIES uploadHeapProp = HeapProperties( D3D12_HEAP_TYPE_UPLOAD );

	// Create upload buffer on CPU
	ID3D12Resource* uploadBuffer;
	CHECK_HRESULT( _device->CreateCommittedResource( &uploadHeapProp,
				   D3D12_HEAP_FLAG_NONE,
				   &uploadBufferDesc,
				   D3D12_RESOURCE_STATE_GENERIC_READ,
				   nullptr,
				   IID_PPV_ARGS( &uploadBuffer ) ),
				   L"Failed to create upload buffer." );

	if( data )
	{
		CopyMemoryToBuffer( uploadBuffer, data, sizeInBytes );
	}

	CHECK_HRESULT( cmdAllocator->Reset(), L"Failed to reset command allocator." );
	CHECK_HRESULT( cmdList->Reset( cmdAllocator, nullptr ), L"Failed to reset command list." );

	TransitionResource( cmdList, resource.resource, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST );

	cmdList->CopyBufferRegion( resource.resource, 0, uploadBuffer, 0, sizeInBytes );

	TransitionResource( cmdList, resource.resource, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COMMON );

	CHECK_HRESULT( cmdList->Close(), L"Failed to close command list." );

	ID3D12CommandList* cmdListInterface = cmdList;
	cmdQueueCtx.cmdQueue->ExecuteCommandLists( 1, &cmdListInterface );

	WaitForFence( cmdQueueCtx );

	uploadBuffer->Release();
}

void GPI_DX12::TransitionResource( ID3D12GraphicsCommandList* cmdList, ID3D12Resource* resource, D3D12_RESOURCE_STATES stateBefore, D3D12_RESOURCE_STATES stateAfter )
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

void GPI_DX12::TransitionResource( const IGPIResource& inResource, const EGPIResourceStates statesBefore, const EGPIResourceStates statesAfter )
{
	CommandQueueContext& cmdQueueCtx = _cmdQueueCtx[ D3D12_COMMAND_LIST_TYPE_DIRECT ];
	ID3D12GraphicsCommandList* cmdList = *cmdQueueCtx.iCmdList;
	const GPIResource_DX12& resource = static_cast< const GPIResource_DX12& >( inResource );

	TransitionResource( cmdList, resource.resource, GPIUtil::TranslateResourceState( statesBefore ), GPIUtil::TranslateResourceState( statesAfter ) );
}

void GPI_DX12::RunCS()
{
	//CHECK_HRESULT( _cmdQueueCtx[ D3D12_COMMAND_LIST_TYPE_COMPUTE ].allocator->Reset(), L"Failed to reset command allocator." );
	//CHECK_HRESULT( ( *_cmdQueueCtx[ D3D12_COMMAND_LIST_TYPE_COMPUTE ].iCmdList )->Reset( _cmdQueueCtx[ D3D12_COMMAND_LIST_TYPE_COMPUTE ].allocator, nullptr ), L"Failed to reset command list." );

	//static ID3D12Resource* buffer = nullptr;
	//if( !buffer )
	//{
	//	_guavHeap = CreateDescriptorHeap( _device, 4, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE );

	//	int32 data[ 64 ] = {};
	//	buffer = CreateBuffer( _device, _cmdQueueCtx[ D3D12_COMMAND_LIST_TYPE_COPY ], data, 256 );

	//	TransitionResource( *_cmdQueueCtx[ D3D12_COMMAND_LIST_TYPE_COMPUTE ].iCmdList, buffer, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS );

	//	D3D12_CPU_DESCRIPTOR_HANDLE uavDescHandle = _guavHeap->GetCPUDescriptorHandleForHeapStart();

	//	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
	//	uavDesc.Format = DXGI_FORMAT_R32_UINT;
	//	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	//	uavDesc.Buffer.FirstElement = 0;
	//	uavDesc.Buffer.CounterOffsetInBytes = 0;
	//	uavDesc.Buffer.NumElements = 64;
	//	uavDesc.Buffer.StructureByteStride = 0;
	//	uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

	//	_device->CreateUnorderedAccessView( buffer, nullptr, &uavDesc, uavDescHandle );
	//}

	//static ID3D12RootSignature* rootSignature = nullptr;
	//static ID3D12PipelineState* pipelineState = nullptr;
	//if( !rootSignature )
	//{
	//	// temp
	//	//rootSignature = CreateRootSignature1();
	//	//pipelineState = CreatePipelineState1( rootSignature );
	//}

	//CommandQueueContext& cmdQueueCtx = _cmdQueueCtx[ D3D12_COMMAND_LIST_TYPE_COMPUTE ];
	//ID3D12GraphicsCommandList* cmdList = *cmdQueueCtx.iCmdList;

	//cmdList->SetPipelineState( pipelineState );
	//cmdList->SetComputeRootSignature( rootSignature );

	//cmdList->SetDescriptorHeaps( 1, &_guavHeap );
	//cmdList->SetComputeRootDescriptorTable( 0, _guavHeap->GetGPUDescriptorHandleForHeapStart() );

	//cmdList->Dispatch( 1, 1, 1 );

	//CHECK_HRESULT( cmdList->Close(), L"Failed to close command list." );

	//ID3D12Fence* fence = cmdQueueCtx.fence;
	//HANDLE fenceEventHandle = cmdQueueCtx.fenceEventHandle;
	//ID3D12CommandList* cmdListInterface = cmdList;
	//cmdQueueCtx.cmdQueue->ExecuteCommandLists( 1, &cmdListInterface );

	//WaitForFence( cmdQueueCtx );
}