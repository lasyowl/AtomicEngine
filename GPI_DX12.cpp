#include "stdafx.h"
#include "GPI_DX12.h"
#include "EngineDefines.h"

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

/* todo : Generate from file */
std::tuple<ID3D12PipelineState*, ID3D12RootSignature*> CreatePSO( ID3D12Device* device )
{
	ID3D12PipelineState* pso;
	ID3D12RootSignature* rootSignature;

	/* Create root signature */
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

	CHECK_HRESULT( device->CreateRootSignature( 0, rootBlob->GetBufferPointer(), rootBlob->GetBufferSize(), IID_PPV_ARGS( &rootSignature ) ),
				   "Failed to create root signature" );

	/* Create pipeline state object */
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
	psoDesc.InputLayout.NumElements = std::extent<decltype(layout)>::value;
	psoDesc.InputLayout.pInputElementDescs = layout;
	psoDesc.SampleDesc.Count = 1;
	psoDesc.DepthStencilState.DepthEnable = false;
	psoDesc.DepthStencilState.StencilEnable = false;
	psoDesc.SampleMask = 0xFFFFFFFF;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	device->CreateGraphicsPipelineState( &psoDesc, IID_PPV_ARGS( &pso ) );

	return { pso, rootSignature };
}

IVertexBufferContextRef CreateVertexBuffer( void* data, uint64 SizeInByte )
{
	return CreateBuffer< SVertexBufferContextDX12 >( data, SizeInByte );
}

IIndexBufferContextRef CreateIndexBuffer( void* data, uint64 SizeInByte )
{
	return CreateBuffer< SIndexBufferContextDX12 >( data, SizeInByte );
}

template<typename TBufferContext>
std::shared_ptr<TBufferContext> CreateBuffer( void* data, uint64 SizeInByte )
{
	CDX12CommandQueueSet& CmdQueueSet = CmdQueueSets[ D3D12_COMMAND_LIST_TYPE_COPY ];
	ID3D12GraphicsCommandList* CmdList = *CmdQueueSet.CmdListIter;
	ID3D12CommandAllocator* CmdAllocator = CmdQueueSet.Allocator;
	ID3D12CommandQueue* CmdQueue = CmdQueueSet.CmdQueue;
	ID3D12Fence* Fence = CmdQueueSet.Fence;
	HANDLE FenceEventHandle = CmdQueueSet.FenceEventHandle;

	D3D12_RESOURCE_DESC BufferDesc = Predefined::ResourceDescBuffer( SizeInByte );
	D3D12_HEAP_PROPERTIES UploadHeapProp = Predefined::HeapProperties( D3D12_HEAP_TYPE_UPLOAD );
	D3D12_HEAP_PROPERTIES DefaultHeapProp = Predefined::HeapProperties( D3D12_HEAP_TYPE_DEFAULT );

	// Create upload buffer on CPU
	ID3D12Resource* UploadBuffer;
	CHECK_HRESULT( Device->CreateCommittedResource( &UploadHeapProp,
				   D3D12_HEAP_FLAG_NONE,
				   &BufferDesc,
				   D3D12_RESOURCE_STATE_GENERIC_READ,
				   nullptr,
				   IID_PPV_ARGS( &UploadBuffer ) ) );

	ID3D12Resource* OutBuffer;
	CHECK_HRESULT( Device->CreateCommittedResource( &DefaultHeapProp,
				   D3D12_HEAP_FLAG_NONE,
				   &BufferDesc,
				   D3D12_RESOURCE_STATE_COPY_DEST,
				   nullptr,
				   IID_PPV_ARGS( &OutBuffer ) ) );

	CopyMemoryToBuffer( UploadBuffer, data, SizeInByte );

	CHECK_HRESULT( CmdAllocator->Reset() );
	CHECK_HRESULT( CmdList->Reset( CmdAllocator, nullptr ) );

	CmdList->CopyBufferRegion( OutBuffer, 0, UploadBuffer, 0, SizeInByte );

	D3D12_RESOURCE_BARRIER Barrier;
	Barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	Barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	Barrier.Transition.pResource = OutBuffer;
	Barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	Barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COMMON;
	Barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	CmdList->ResourceBarrier( 1, &Barrier );

	CHECK_HRESULT( CmdList->Close() );

	CmdQueueSet.ExecuteCommandList( CmdList );
	CHECK_HRESULT( CmdQueue->Signal( Fence, 1 ) );

	if( Fence->GetCompletedValue() != 1 )
	{
		Fence->SetEventOnCompletion( 1, FenceEventHandle );
		WaitForSingleObject( FenceEventHandle, INFINITE );
	}

	return std::make_shared<TBufferContext>( OutBuffer );
}

void CopyMemoryToBuffer( ID3D12Resource * Buffer, void* data, uint64 SizeInByte )
{
	void* VirtualMem;
	Buffer->Map( 0, nullptr, &VirtualMem );
	::memcpy( VirtualMem, data, SizeInByte );
	Buffer->Unmap( 0, nullptr );
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
		swapChainBuffer.BufferIndex = Index;

		D3D12_RENDER_TARGET_VIEW_DESC viewDesc{};
		viewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		viewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		viewDesc.Texture2D.MipSlice = 0;
		viewDesc.Texture2D.PlaneSlice = 0;

		CHECK_HRESULT( _swapChain->GetBuffer( Index, IID_PPV_ARGS( &swapChainBuffer.RenderTarget ) ), "Failed to get swapchain buffer." );

		_device->CreateRenderTargetView( swapChainBuffer.RenderTarget, &viewDesc, RenderTargetViewDescHandle );

		swapChainBuffer.RenderTargetHandlePtr = RenderTargetViewDescHandle.ptr;
		RenderTargetViewDescHandle.ptr += RenderTargetViewDescSize;
	}

	_swapChainBufferIter = _swapChainBuffers.begin();
}

void GPI_DX12::BeginFrame()
{
	CDX12CommandQueueSet& CmdQueueSet = CmdQueueSets[ D3D12_COMMAND_LIST_TYPE_DIRECT ];
	ID3D12CommandAllocator* CmdAllocator = CmdQueueSet.Allocator;
	ID3D12GraphicsCommandList* CmdList = *CmdQueueSet.CmdListIter;
	ID3D12CommandQueue* CmdQueue = CmdQueueSet.CmdQueue;
	ID3D12Fence* Fence = CmdQueueSet.Fence;
	uint64& FenceValue = CmdQueueSet.FenceValue;
	HANDLE FenceEventHandle = CmdQueueSet.FenceEventHandle;

	if( Fence->GetCompletedValue() < FenceValue )
	{
		CHECK_HRESULT( Fence->SetEventOnCompletion( FenceValue, FenceEventHandle ) );
		WaitForSingleObject( FenceEventHandle, INFINITE );
	}

	CHECK_HRESULT( CmdAllocator->Reset() );
	CHECK_HRESULT( CmdList->Reset( CmdAllocator, nullptr ) );

	D3D12_CPU_DESCRIPTOR_HANDLE RenderTargetHandle;
	RenderTargetHandle.ptr = SwapChainBufferIter->RenderTargetHandlePtr;

	CmdList->OMSetRenderTargets( 1, &RenderTargetHandle, true, nullptr );
	CmdList->RSSetViewports( 1, &WindowViewport );
	CmdList->RSSetScissorRects( 1, &WindowScissorRect );

	// Transition back buffer
	D3D12_RESOURCE_BARRIER Barrier;
	Barrier.Transition.pResource = SwapChainBufferIter->RenderTarget;
	Barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	Barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	Barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	Barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	Barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	CmdList->ResourceBarrier( 1, &Barrier );

	static const float ClearColor[] = {
		0.042f, 0.042f, 0.242f,	1
	};

	CmdList->ClearRenderTargetView( RenderTargetHandle,
									ClearColor, 0, nullptr );

	CHECK_HRESULT( CmdList->Close() );

	CmdQueueSet.ExecuteCommandList( CmdList );
	CmdQueue->Signal( Fence, ++FenceValue );
}

void CGraphicsInterfaceDX12::EndFrame()
{
	CDX12CommandQueueSet& CmdQueueSet = CmdQueueSets[ D3D12_COMMAND_LIST_TYPE_DIRECT ];
	ID3D12GraphicsCommandList* CmdList = *CmdQueueSet.CmdListIter;
	ID3D12CommandAllocator* CmdAllocator = CmdQueueSet.Allocator;
	ID3D12CommandQueue* CmdQueue = CmdQueueSet.CmdQueue;
	ID3D12Fence* Fence = CmdQueueSet.Fence;
	uint64& FenceValue = CmdQueueSet.FenceValue;
	HANDLE FenceEventHandle = CmdQueueSet.FenceEventHandle;

	if( Fence->GetCompletedValue() < FenceValue )
	{
		CHECK_HRESULT( Fence->SetEventOnCompletion( FenceValue, FenceEventHandle ) );
		WaitForSingleObject( FenceEventHandle, INFINITE );
	}

	CHECK_HRESULT( CmdAllocator->Reset() );
	CHECK_HRESULT( CmdList->Reset( CmdAllocator, nullptr ) );

	D3D12_RESOURCE_BARRIER Barrier;
	Barrier.Transition.pResource = SwapChainBufferIter->RenderTarget;
	Barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	Barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	Barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	Barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	Barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	CmdList->ResourceBarrier( 1, &Barrier );

	CHECK_HRESULT( CmdList->Close() );

	CmdQueueSet.ExecuteCommandList( CmdList );

	CHECK_HRESULT( SwapChain->Present( 1, 0 ) );

	CHECK_HRESULT( CmdQueue->Signal( Fence, ++FenceValue ) );

	if( ++SwapChainBufferIter == SwapChainBuffers.end() )
	{
		SwapChainBufferIter = SwapChainBuffers.begin();
	}

	CmdQueueSet.ItinerateCommandList();
}

void CGraphicsInterfaceDX12::SetPipelineState( ID3D12PipelineState* pso, ID3D12RootSignature* rootSignature )
{
	CDX12CommandQueueSet& CmdQueueSet = CmdQueueSets[ D3D12_COMMAND_LIST_TYPE_DIRECT ];
	ID3D12GraphicsCommandList* CmdList = *CmdQueueSet.CmdListIter;
	ID3D12CommandAllocator* CmdAllocator = CmdQueueSet.Allocator;
	ID3D12Fence* Fence = CmdQueueSet.Fence;
	uint64& FenceValue = CmdQueueSet.FenceValue;
	HANDLE FenceEventHandle = CmdQueueSet.FenceEventHandle;

	if( Fence->GetCompletedValue() < FenceValue )
	{
		CHECK_HRESULT( Fence->SetEventOnCompletion( FenceValue, FenceEventHandle ), 
					   "Failed to set fence." );
		WaitForSingleObject( FenceEventHandle, INFINITE );
	}

	CHECK_HRESULT( CmdAllocator->Reset(), "Failed to reset allocator." );
	CHECK_HRESULT( CmdList->Reset( CmdAllocator, nullptr ), "Failed to reset command list." );

	CmdList->SetPipelineState( pso );
	CmdList->SetGraphicsRootSignature( rootSignature );
	CmdList->IASetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

	D3D12_CPU_DESCRIPTOR_HANDLE RenderTargetHandle;
	RenderTargetHandle.ptr = SwapChainBufferIter->RenderTargetHandlePtr;

	CmdList->OMSetRenderTargets( 1, &RenderTargetHandle, true, nullptr );
	CmdList->RSSetViewports( 1, &WindowViewport );
	CmdList->RSSetScissorRects( 1, &WindowScissorRect );
}

void CGraphicsInterfaceDX12::Render( const CRenderObject& InRenderObject )
{
	CDX12CommandQueueSet& CmdQueueSet = CmdQueueSets[ D3D12_COMMAND_LIST_TYPE_DIRECT ];
	ID3D12GraphicsCommandList* CmdList = *CmdQueueSet.CmdListIter;

	const CVertexBuffer& VB = InRenderObject.GetVertexBuffer();
	const CIndexBuffer& IB = InRenderObject.GetIndexBuffer();

	D3D12_VERTEX_BUFFER_VIEW VBView;
	VBView.BufferLocation = VB.GetGPUVirtualAddress();
	VBView.SizeInBytes = (uint32) VB.GetSizeInByte();
	VBView.StrideInBytes = VB.GetStrideInByte();

	D3D12_INDEX_BUFFER_VIEW IBView;
	IBView.BufferLocation = IB.GetGPUVirtualAddress();
	IBView.SizeInBytes = (uint32) IB.GetSizeInByte();
	IBView.Format = DXGI_FORMAT_R32_UINT;

	CmdList->IASetVertexBuffers( 0, 1, &VBView );
	CmdList->IASetIndexBuffer( &IBView );
	CmdList->DrawIndexedInstanced( 6, 1, 0, 0, 0 );
}

void CGraphicsInterfaceDX12::FlushPipelineState()
{
	CDX12CommandQueueSet& CmdQueueSet = CmdQueueSets[ D3D12_COMMAND_LIST_TYPE_DIRECT ];
	ID3D12GraphicsCommandList* CmdList = *CmdQueueSet.CmdListIter;
	ID3D12CommandQueue* CmdQueue = CmdQueueSet.CmdQueue;
	ID3D12Fence* Fence = CmdQueueSet.Fence;
	uint64& FenceValue = CmdQueueSet.FenceValue;

	CHECK_HRESULT( CmdList->Close(), "Failed to close command list." );

	CmdQueueSet.ExecuteCommandList( CmdList );

	CHECK_HRESULT( CmdQueue->Signal( Fence, ++FenceValue ), "Failed to signal fence." );
}
