#include "stdafx.h"
#include "GPI_DX12.h"
#include "EngineDefines.h"
#include "AtomicEngine.h"

#include <d3d12.h>
#include <dxgi.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <comdef.h>
#include <fstream>

#define CHECK_HRESULT( hr, msg ) \
	if(FAILED (hr)) {\
		throw std::runtime_error( msg );\
	}

constexpr D3D12_VIEWPORT ToDX12Viewport( uint32 width, uint32 height )
{
	return D3D12_VIEWPORT{ 0, 0, ( float )width, ( float )height, 0, 1 };
}

constexpr D3D12_RECT ToDX12Rect( uint32 width, uint32 height )
{
	return D3D12_RECT{ 0, 0, ( int32 )width, ( int32 )height };
}

constexpr D3D12_RESOURCE_DESC ResourceDescBuffer( uint64 width, uint32 height = 1, uint16 depth = 1 )
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

void CopyMemoryToBuffer( ID3D12Resource* buffer, void* data, uint64 size )
{
	void* virtualMem;
	buffer->Map( 0, nullptr, &virtualMem );
	::memcpy( virtualMem, data, size );
	buffer->Unmap( 0, nullptr );
}

ID3D12Resource* CreateBuffer( ID3D12Device* device, CommandQueueContext& cmdQueueContext, void* data, uint64 size )
{
	ID3D12GraphicsCommandList* cmdList = *cmdQueueContext.cmdListIter;
	ID3D12CommandAllocator* cmdAllocator = cmdQueueContext.allocator;
	ID3D12Fence* fence = cmdQueueContext.fence;
	HANDLE fenceEventHandle = cmdQueueContext.fenceEventHandle;

	D3D12_RESOURCE_DESC bufferDesc = ResourceDescBuffer( size );
	D3D12_HEAP_PROPERTIES uploadHeapProp = HeapProperties( D3D12_HEAP_TYPE_UPLOAD );
	D3D12_HEAP_PROPERTIES defaultHeapProp = HeapProperties( D3D12_HEAP_TYPE_DEFAULT );

	// Create upload buffer on CPU
	ID3D12Resource* uploadBuffer;
	CHECK_HRESULT( device->CreateCommittedResource( &uploadHeapProp,
				   D3D12_HEAP_FLAG_NONE,
				   &bufferDesc,
				   D3D12_RESOURCE_STATE_GENERIC_READ,
				   nullptr,
				   IID_PPV_ARGS( &uploadBuffer ) ),
				   "Failed to create upload buffer." );

	ID3D12Resource* outBuffer;
	CHECK_HRESULT( device->CreateCommittedResource( &defaultHeapProp,
				   D3D12_HEAP_FLAG_NONE,
				   &bufferDesc,
				   D3D12_RESOURCE_STATE_COPY_DEST,
				   nullptr,
				   IID_PPV_ARGS( &outBuffer ) ),
				   "Failed to create output buffer." );

	CopyMemoryToBuffer( uploadBuffer, data, size );

	CHECK_HRESULT( cmdAllocator->Reset(), "Failed to reset command allocator." );
	CHECK_HRESULT( cmdList->Reset( cmdAllocator, nullptr ), "Failed to reset command list." );

	cmdList->CopyBufferRegion( outBuffer, 0, uploadBuffer, 0, size );

	D3D12_RESOURCE_BARRIER barrier;
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = outBuffer;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COMMON;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	cmdList->ResourceBarrier( 1, &barrier );

	CHECK_HRESULT( cmdList->Close(), "Failed to close command list." );

	ID3D12CommandList* cmdListInterface = cmdList;
	cmdQueueContext.cmdQueue->ExecuteCommandLists( 1, &cmdListInterface );
	CHECK_HRESULT( cmdQueueContext.cmdQueue->Signal( fence, 1 ), "Failed to signal command queue." );

	if( fence->GetCompletedValue() != 1 )
	{
		fence->SetEventOnCompletion( 1, fenceEventHandle );
		WaitForSingleObject( fenceEventHandle, INFINITE );
	}

	return outBuffer;
}

///////////////////////////////////////
// GPI_DX12
///////////////////////////////////////
GPI_DX12::GPI_DX12( const HWND hWnd, const int32 screenWidth, const int32 screenHeight )
	: _device( nullptr )
	, _swapChain( nullptr )
	, _descHeap( nullptr )
	, _debugInterface( nullptr )
	, _debugInfoQueue( nullptr )
	, _hWnd( hWnd )
	, _screenWidth( screenWidth )
	, _screenHeight( screenHeight )
{}

void GPI_DX12::Initialize()
{
	/* Create device */
	CHECK_HRESULT( D3D12CreateDevice( nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS( &_device ) ), "Failed to create device." );

	/* Cretate debug layer */
	CHECK_HRESULT( D3D12GetDebugInterface( IID_PPV_ARGS( &_debugInterface ) ), "Failed to create debug layer.");

	/* Create command queue and lists */
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
					   "Failed to create command queue." );
		CHECK_HRESULT( _device->CreateCommandAllocator( queueDesc.Type, IID_PPV_ARGS( &cmdQueueCtx.allocator ) ), 
					   "Failed to create command allocator." );

		for( int32 Index = 0; Index < CMD_LIST_PER_QUEUE_COUNT; ++Index )
		{
			ID3D12GraphicsCommandList*& cmdList = cmdQueueCtx.cmdLists[ Index ];

			CHECK_HRESULT( _device->CreateCommandList( 0, queueDesc.Type, cmdQueueCtx.allocator, nullptr, IID_PPV_ARGS( &cmdList ) ), 
						   "Failed to create command list." );
			CHECK_HRESULT( cmdList->Close(), 
						   "Failed to close command list." );
		}

		CHECK_HRESULT( _device->CreateFence( 0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS( &cmdQueueCtx.fence ) ), 
					   "Failed to create fence.");

		cmdQueueCtx.fenceEventHandle = CreateEvent( nullptr, FALSE, FALSE, nullptr );
		cmdQueueCtx.fenceValue = 0;
		cmdQueueCtx.cmdListIter = cmdQueueCtx.cmdLists.begin();
	}

	/* Create swapchain */
	CommandQueueContext& cmdQueueCtx = _cmdQueueCtx[ D3D12_COMMAND_LIST_TYPE_DIRECT ];

	DXGI_SWAP_CHAIN_DESC swapChainDesc{};
	swapChainDesc.BufferCount = SWAPCHAIN_BUFFER_COUNT;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferDesc.Width = _screenWidth;
	swapChainDesc.BufferDesc.Height = _screenHeight;
	swapChainDesc.OutputWindow = _hWnd;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.Windowed = true;

	IDXGIFactory6* dxgiFactory;
	CHECK_HRESULT( CreateDXGIFactory1( IID_PPV_ARGS( &dxgiFactory ) ), 
				   "Failed to create dxgi factory." );
	CHECK_HRESULT( dxgiFactory->CreateSwapChain( cmdQueueCtx.cmdQueue, &swapChainDesc, &_swapChain ), 
				   "Failed to create swapchain." );

	/* Create descriptor heap */
	D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc{};
	descHeapDesc.NumDescriptors = SWAPCHAIN_BUFFER_COUNT;
	descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	CHECK_HRESULT( _device->CreateDescriptorHeap( &descHeapDesc, IID_PPV_ARGS( &_descHeap ) ), 
				   "Failed to create descriptor heap." );

	/* Create rendertargets */
	D3D12_CPU_DESCRIPTOR_HANDLE RenderTargetViewDescHandle = _descHeap->GetCPUDescriptorHandleForHeapStart();
	size_t RenderTargetViewDescSize = _device->GetDescriptorHandleIncrementSize( D3D12_DESCRIPTOR_HEAP_TYPE_RTV );

	for( int32 Index = 0; Index < SWAPCHAIN_BUFFER_COUNT; ++Index )
	{
		SwapChainBufferContext& swapChainBuffer = _swapChainBuffers[ Index ];
		swapChainBuffer.bufferIndex = Index;

		D3D12_RENDER_TARGET_VIEW_DESC viewDesc{};
		viewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		viewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		viewDesc.Texture2D.MipSlice = 0;
		viewDesc.Texture2D.PlaneSlice = 0;

		CHECK_HRESULT( _swapChain->GetBuffer( Index, IID_PPV_ARGS( &swapChainBuffer.renderTarget ) ), "Failed to get swapchain buffer." );

		_device->CreateRenderTargetView( swapChainBuffer.renderTarget, &viewDesc, RenderTargetViewDescHandle );

		swapChainBuffer.renderTargetHandlePtr = RenderTargetViewDescHandle.ptr;
		RenderTargetViewDescHandle.ptr += RenderTargetViewDescSize;
	}

	_swapChainBufferIter = _swapChainBuffers.begin();
}

void GPI_DX12::BeginFrame()
{
	CommandQueueContext& cmdQueueCtx = _cmdQueueCtx[ D3D12_COMMAND_LIST_TYPE_DIRECT ];
	ID3D12CommandAllocator* cmdAllocator = cmdQueueCtx.allocator;
	ID3D12GraphicsCommandList* cmdList = *cmdQueueCtx.cmdListIter;
	ID3D12CommandQueue* cmdQueue = cmdQueueCtx.cmdQueue;
	ID3D12Fence* fence = cmdQueueCtx.fence;
	uint64& fenceValue = cmdQueueCtx.fenceValue;
	HANDLE fenceEventHandle = cmdQueueCtx.fenceEventHandle;

	if( fence->GetCompletedValue() < fenceValue )
	{
		CHECK_HRESULT( fence->SetEventOnCompletion( fenceValue, fenceEventHandle ),
					   "Failed to set fence." );
		WaitForSingleObject( fenceEventHandle, INFINITE );
	}

	CHECK_HRESULT( cmdAllocator->Reset(), "Failed to reset command allocator." );
	CHECK_HRESULT( cmdList->Reset( cmdAllocator, nullptr ), "Failed to reset command list." );

	D3D12_CPU_DESCRIPTOR_HANDLE renderTargetHandle;
	renderTargetHandle.ptr = _swapChainBufferIter->renderTargetHandlePtr;
	cmdList->OMSetRenderTargets( 1, &renderTargetHandle, true, nullptr );

	D3D12_VIEWPORT viewport = ToDX12Viewport( _windowWidth, _windowHeight );
	D3D12_RECT scissorRect = ToDX12Rect( _windowWidth, _windowHeight );
	cmdList->RSSetViewports( 1, &viewport );
	cmdList->RSSetScissorRects( 1, &scissorRect );

	// Transition back buffer
	D3D12_RESOURCE_BARRIER barrier;
	barrier.Transition.pResource = _swapChainBufferIter->renderTarget;
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	cmdList->ResourceBarrier( 1, &barrier );

	static float clearColor[] = {
		0.042f, 0.042f, 0.242f,	1
	};

	cmdList->ClearRenderTargetView( renderTargetHandle, clearColor, 0, nullptr );

	CHECK_HRESULT( cmdList->Close(), "Failed to close command list." );

	ID3D12CommandList* cmdListInterface = cmdList;
	cmdQueue->ExecuteCommandLists( 1, &cmdListInterface );

	cmdQueue->Signal( fence, ++fenceValue );
}

void GPI_DX12::EndFrame()
{
	CommandQueueContext& cmdQueueCtx = _cmdQueueCtx[ D3D12_COMMAND_LIST_TYPE_DIRECT ];
	ID3D12CommandAllocator* cmdAllocator = cmdQueueCtx.allocator;
	ID3D12GraphicsCommandList* cmdList = *cmdQueueCtx.cmdListIter;
	ID3D12CommandQueue* cmdQueue = cmdQueueCtx.cmdQueue;
	ID3D12Fence* fence = cmdQueueCtx.fence;
	uint64& fenceValue = cmdQueueCtx.fenceValue;
	HANDLE fenceEventHandle = cmdQueueCtx.fenceEventHandle;

	if( fence->GetCompletedValue() < fenceValue )
	{
		CHECK_HRESULT( fence->SetEventOnCompletion( fenceValue, fenceEventHandle ),
					   "Failed to set fence." );
		WaitForSingleObject( fenceEventHandle, INFINITE );
	}

	CHECK_HRESULT( cmdAllocator->Reset(), "Failed to reset command allocator." );
	CHECK_HRESULT( cmdList->Reset( cmdAllocator, nullptr ), "Failed to reset command list." );
	
	D3D12_RESOURCE_BARRIER barrier;
	barrier.Transition.pResource = _swapChainBufferIter->renderTarget;
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	cmdList->ResourceBarrier( 1, &barrier );
	
	CHECK_HRESULT( cmdList->Close(), "Failed to close command list." );
	
	ID3D12CommandList* cmdListInterface = cmdList;
	cmdQueue->ExecuteCommandLists( 1, &cmdListInterface );

	CHECK_HRESULT( _swapChain->Present( 1, 0 ), "Failed to present swapchain." );

	CHECK_HRESULT( cmdQueue->Signal( fence, ++fenceValue ), "Failed to signal command queue." );

	if( ++_swapChainBufferIter == _swapChainBuffers.end() )
	{
		_swapChainBufferIter = _swapChainBuffers.begin();
	}

	if( ++cmdQueueCtx.cmdListIter == cmdQueueCtx.cmdLists.end() )
	{
		cmdQueueCtx.cmdListIter = cmdQueueCtx.cmdLists.begin();
	}
}

void GPI_DX12::SetPipelineState( ID3D12PipelineState* pso, ID3D12RootSignature* rootSignature )
{
	CommandQueueContext& cmdQueueCtx = _cmdQueueCtx[ D3D12_COMMAND_LIST_TYPE_DIRECT ];
	ID3D12CommandAllocator* cmdAllocator = cmdQueueCtx.allocator;
	ID3D12GraphicsCommandList* cmdList = *cmdQueueCtx.cmdListIter;
	ID3D12Fence* fence = cmdQueueCtx.fence;
	uint64& fenceValue = cmdQueueCtx.fenceValue;
	HANDLE fenceEventHandle = cmdQueueCtx.fenceEventHandle;

	if( fence->GetCompletedValue() < fenceValue )
	{
		CHECK_HRESULT( fence->SetEventOnCompletion( fenceValue, fenceEventHandle ),
					   "Failed to set fence." );
		WaitForSingleObject( fenceEventHandle, INFINITE );
	}

	CHECK_HRESULT( cmdAllocator->Reset(), "Failed to reset allocator." );
	CHECK_HRESULT( cmdList->Reset( cmdAllocator, nullptr ), "Failed to reset command list." );

	cmdList->SetPipelineState( pso );
	cmdList->SetGraphicsRootSignature( rootSignature );
	cmdList->IASetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

	D3D12_CPU_DESCRIPTOR_HANDLE renderTargetHandle;
	renderTargetHandle.ptr = _swapChainBufferIter->renderTargetHandlePtr;
	cmdList->OMSetRenderTargets( 1, &renderTargetHandle, true, nullptr );

	D3D12_VIEWPORT viewport = ToDX12Viewport( _windowWidth, _windowHeight );
	D3D12_RECT scissorRect = ToDX12Rect( _windowWidth, _windowHeight );
	cmdList->RSSetViewports( 1, &viewport );
	cmdList->RSSetScissorRects( 1, &scissorRect );
}

void GPI_DX12::SetPipelineState( uint32 pipelineStateHash )
{
	assert( _pipelineStateCache.end() != _pipelineStateCache.find( pipelineStateHash ) );

	auto& [rootSignature, pso] = _pipelineStateCache[ pipelineStateHash ];
	SetPipelineState( pso, rootSignature );
}

void GPI_DX12::Render( IVertexBuffer* vertexBuffer, IIndexBuffer* indexBuffer )
{
	CommandQueueContext& cmdQueueCtx = _cmdQueueCtx[ D3D12_COMMAND_LIST_TYPE_DIRECT ];
	ID3D12GraphicsCommandList* cmdList = *cmdQueueCtx.cmdListIter;

	D3D12_VERTEX_BUFFER_VIEW VBView;
	VBView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
	VBView.SizeInBytes = ( uint32 )vertexBuffer->GetSize();
	VBView.StrideInBytes = vertexBuffer->GetStride();

	D3D12_INDEX_BUFFER_VIEW IBView;
	IBView.BufferLocation = indexBuffer->GetGPUVirtualAddress();
	IBView.SizeInBytes = ( uint32 )indexBuffer->GetSize();
	IBView.Format = DXGI_FORMAT_R32_UINT;

	cmdList->IASetVertexBuffers( 0, 1, &VBView );
	cmdList->IASetIndexBuffer( &IBView );
	cmdList->DrawIndexedInstanced( 6, 1, 0, 0, 0 );
}

void GPI_DX12::FlushPipelineState()
{
	CommandQueueContext& cmdQueueCtx = _cmdQueueCtx[ D3D12_COMMAND_LIST_TYPE_DIRECT ];
	ID3D12GraphicsCommandList* cmdList = *cmdQueueCtx.cmdListIter;
	ID3D12CommandQueue* cmdQueue = cmdQueueCtx.cmdQueue;
	ID3D12Fence* fence = cmdQueueCtx.fence;
	uint64& fenceValue = cmdQueueCtx.fenceValue;

	CHECK_HRESULT( cmdList->Close(), "Failed to close command list." );

	ID3D12CommandList* cmdListInterface = cmdList;
	cmdQueue->ExecuteCommandLists( 1, &cmdListInterface );

	CHECK_HRESULT( cmdQueue->Signal( fence, ++fenceValue ), "Failed to signal fence." );
}

/* todo : Generate from file */
ID3D12RootSignature* GPI_DX12::CreateRootSignature()
{
	ID3D12RootSignature* rootSignature;

	D3D12_ROOT_PARAMETER rootParam{};
	rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParam.Descriptor.ShaderRegister = 0;
	rootParam.Descriptor.RegisterSpace = 0;

	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
	rootSignatureDesc.NumParameters = 1;
	rootSignatureDesc.pParameters = &rootParam;
	rootSignatureDesc.NumStaticSamplers = 0;
	rootSignatureDesc.pStaticSamplers = nullptr;
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	ID3DBlob* rootBlob;
	ID3DBlob* errorBlob;
	CHECK_HRESULT( D3D12SerializeRootSignature( &rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &rootBlob, &errorBlob ),
				   "Failed to serialize root signature" );

	CHECK_HRESULT( _device->CreateRootSignature( 0, rootBlob->GetBufferPointer(), rootBlob->GetBufferSize(), IID_PPV_ARGS( &rootSignature ) ),
				   "Failed to create root signature" );

	return rootSignature;
}

/* todo : Generate from file */
ID3D12PipelineState* GPI_DX12::CreatePipelineState( ID3D12RootSignature* rootSignature )
{
	ID3D12PipelineState* pso;

	const D3D12_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
		D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12,
		D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	const D3D_SHADER_MACRO macros[] = {
		{ "D3D12_SAMPLE_BASIC", "1" },
		{ nullptr, nullptr }
	};

	std::ifstream shaderFile( "Engine/Shader/Test.hlsl", std::ios_base::in );
	std::string parsedShader = std::string( std::istreambuf_iterator<char>( shaderFile ),
											std::istreambuf_iterator<char>() );

	ID3DBlob* vertexShader;
	CHECK_HRESULT( D3DCompile( parsedShader.c_str(), parsedShader.size(),
				   "", macros, nullptr,
				   "VS_main", "vs_5_0", 0, 0, &vertexShader, nullptr ),
				   "Vertex shader compilation failed." );

	ID3DBlob* pixelShader;
	CHECK_HRESULT( D3DCompile( parsedShader.c_str(), parsedShader.size(),
				   "", macros, nullptr,
				   "PS_main", "ps_5_0", 0, 0, &pixelShader, nullptr ),
				   "Pixel shader compilation failed." );

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
	blendState.RenderTarget[ 0 ].BlendEnable = true;
	blendState.RenderTarget[ 0 ].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	blendState.RenderTarget[ 0 ].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	blendState.RenderTarget[ 0 ].BlendOp = D3D12_BLEND_OP_ADD;
	blendState.RenderTarget[ 0 ].SrcBlendAlpha = D3D12_BLEND_ONE;
	blendState.RenderTarget[ 0 ].DestBlendAlpha = D3D12_BLEND_ZERO;
	blendState.RenderTarget[ 0 ].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blendState.RenderTarget[ 0 ].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
	psoDesc.RasterizerState = rasterizerDesc;
	psoDesc.BlendState = blendState;
	psoDesc.VS.BytecodeLength = vertexShader->GetBufferSize();
	psoDesc.VS.pShaderBytecode = vertexShader->GetBufferPointer();
	psoDesc.PS.BytecodeLength = pixelShader->GetBufferSize();
	psoDesc.PS.pShaderBytecode = pixelShader->GetBufferPointer();
	psoDesc.pRootSignature = rootSignature;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[ 0 ] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	psoDesc.DSVFormat = DXGI_FORMAT_UNKNOWN;
	psoDesc.InputLayout.NumElements = std::extent<decltype( layout )>::value;
	psoDesc.InputLayout.pInputElementDescs = layout;
	psoDesc.SampleDesc.Count = 1;
	psoDesc.DepthStencilState.DepthEnable = false;
	psoDesc.DepthStencilState.StencilEnable = false;
	psoDesc.SampleMask = 0xFFFFFFFF;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	_device->CreateGraphicsPipelineState( &psoDesc, IID_PPV_ARGS( &pso ) );

	return pso;
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

uint32 GPI_DX12::CreatePipelineState()
{
	// temp
	ID3D12RootSignature* rootSignature = CreateRootSignature();
	ID3D12PipelineState* pipelineState = CreatePipelineState( rootSignature );
	uint32 hash = reinterpret_cast< uint32 >( rootSignature ) + reinterpret_cast< uint32 >( pipelineState );

	_pipelineStateCache.emplace( hash, std::tuple( rootSignature, pipelineState ) );

	return hash;
}
