#include "stdafx.h"
#include "GraphicsInterfaceDX12.h"

#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <comdef.h>
#include <fstream>

#include "RenderObject.h"
#include "PipelineStateObject.h"
#include "RenderResourceDX12.h"
#include "DX12Misc.h"
#include "WindowsTranslator.h"

#define CHECK_HRESULT( hr ) \
	if (FAILED (hr)) {\
		throw std::runtime_error( "Device creation failed." );\
	}

void CDX12CommandQueueSet::ItinerateCommandList()
{
	if ( ++CmdListIter == CmdLists.end() )
	{
		CmdListIter = CmdLists.begin();
	}
}

void CDX12CommandQueueSet::ExecuteCommandList( ID3D12CommandList* CmdList )
{
	CmdQueue->ExecuteCommandLists( 1, &CmdList );
}

CGraphicsInterfaceDX12::CGraphicsInterfaceDX12()
	: Device( nullptr )
	, SwapChain( nullptr )
	, DescHeap( nullptr )
{}

void CGraphicsInterfaceDX12::InitGraphics( const SGraphicsInitParam& InParam )
{
	SetWindowParams( ( const SGraphicsInitParamDX12& ) InParam );

	CreateDebugLayer();
	CreateDevice();
	CreateDebugLayer();
	CreateCommandQueuesAndLists();
	CreateSwapChain();
	CreateDescriptorHeap();
	CreateRenderTargets();
}

void CGraphicsInterfaceDX12::BeginFrame()
{
	CDX12CommandQueueSet& CmdQueueSet = CmdQueueSets[ D3D12_COMMAND_LIST_TYPE_DIRECT ];
	ID3D12CommandAllocator* CmdAllocator = CmdQueueSet.Allocator;
	ID3D12GraphicsCommandList* CmdList = *CmdQueueSet.CmdListIter;
	ID3D12CommandQueue* CmdQueue = CmdQueueSet.CmdQueue;
	ID3D12Fence* Fence = CmdQueueSet.Fence;
	uint64& FenceValue = CmdQueueSet.FenceValue;
	HANDLE FenceEventHandle = CmdQueueSet.FenceEventHandle;

	if ( Fence->GetCompletedValue() < FenceValue )
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

	if ( Fence->GetCompletedValue() < FenceValue )
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

	if ( ++SwapChainBufferIter == SwapChainBuffers.end() )
	{
		SwapChainBufferIter = SwapChainBuffers.begin();
	}

	CmdQueueSet.ItinerateCommandList();
}

void CGraphicsInterfaceDX12::SetPipelineState( IPipelineStateRef& InPipelineState )
{
	CDX12CommandQueueSet& CmdQueueSet = CmdQueueSets[ D3D12_COMMAND_LIST_TYPE_DIRECT ];
	ID3D12GraphicsCommandList* CmdList = *CmdQueueSet.CmdListIter;
	ID3D12CommandAllocator* CmdAllocator = CmdQueueSet.Allocator;
	ID3D12Fence* Fence = CmdQueueSet.Fence;
	uint64& FenceValue = CmdQueueSet.FenceValue;
	HANDLE FenceEventHandle = CmdQueueSet.FenceEventHandle;

	if ( Fence->GetCompletedValue() < FenceValue )
	{
		CHECK_HRESULT( Fence->SetEventOnCompletion( FenceValue, FenceEventHandle ) );
		WaitForSingleObject( FenceEventHandle, INFINITE );
	}

	CHECK_HRESULT( CmdAllocator->Reset() );
	CHECK_HRESULT( CmdList->Reset( CmdAllocator, nullptr ) );

	CmdList->SetPipelineState( ( ID3D12PipelineState* ) InPipelineState->GetPipelineStateObject() );
	CmdList->SetGraphicsRootSignature( ( ID3D12RootSignature* ) InPipelineState->GetRootSignature() );
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
	VBView.SizeInBytes = ( uint32 ) VB.GetSizeInByte();
	VBView.StrideInBytes = VB.GetStrideInByte();

	D3D12_INDEX_BUFFER_VIEW IBView;
	IBView.BufferLocation = IB.GetGPUVirtualAddress();
	IBView.SizeInBytes = ( uint32 ) IB.GetSizeInByte();
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

	CHECK_HRESULT( CmdList->Close() );

	CmdQueueSet.ExecuteCommandList( CmdList );

	CHECK_HRESULT( CmdQueue->Signal( Fence, ++FenceValue ) );
}

/* todo : Generate from file */
IPipelineStateRef CGraphicsInterfaceDX12::CreatePipelineState()
{
	ID3D12PipelineState* PSO;
	ID3D12RootSignature* RootSignature;

	/* Create root signature begin */
	D3D12_ROOT_PARAMETER RootParam{};
	RootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	RootParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	RootParam.Descriptor.ShaderRegister = 0;
	RootParam.Descriptor.RegisterSpace = 0;

	D3D12_ROOT_SIGNATURE_DESC RootSignatureDesc{};
	RootSignatureDesc.NumParameters = 1;
	RootSignatureDesc.pParameters = &RootParam;
	RootSignatureDesc.NumStaticSamplers = 0;
	RootSignatureDesc.pStaticSamplers = nullptr;
	RootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	ID3DBlob* RootBlob;
	ID3DBlob* ErrorBlob;
	CHECK_HRESULT( D3D12SerializeRootSignature( &RootSignatureDesc,
				   D3D_ROOT_SIGNATURE_VERSION_1, &RootBlob, &ErrorBlob ) );

	CHECK_HRESULT( Device->CreateRootSignature( 0, RootBlob->GetBufferPointer(),
				   RootBlob->GetBufferSize(), IID_PPV_ARGS( &RootSignature ) ) );
	/* Create root signature end */

	const D3D12_INPUT_ELEMENT_DESC Layout[] =
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

	std::ifstream ShaderFile( "Engine/Shader/Test.hlsl", std::ios_base::in );
	std::string ParsedShader = std::string( std::istreambuf_iterator<char>( ShaderFile ),
											std::istreambuf_iterator<char>() );

	ID3DBlob* VertexShader;
	CHECK_HRESULT( D3DCompile( ParsedShader.c_str(), ParsedShader.size(),
				   "", macros, nullptr,
				   "VS_main", "vs_5_0", 0, 0, &VertexShader, nullptr ) );

	ID3DBlob* PixelShader;
	CHECK_HRESULT( D3DCompile( ParsedShader.c_str(), ParsedShader.size(),
				   "", macros, nullptr,
				   "PS_main", "ps_5_0", 0, 0, &PixelShader, nullptr ) );

	D3D12_RASTERIZER_DESC RasterizerDesc{};
	RasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	RasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	RasterizerDesc.FrontCounterClockwise = FALSE;
	RasterizerDesc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
	RasterizerDesc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
	RasterizerDesc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
	RasterizerDesc.DepthClipEnable = TRUE;
	RasterizerDesc.MultisampleEnable = FALSE;
	RasterizerDesc.AntialiasedLineEnable = FALSE;
	RasterizerDesc.ForcedSampleCount = 0;
	RasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	D3D12_BLEND_DESC BlendState{};
	BlendState.AlphaToCoverageEnable = FALSE;
	BlendState.IndependentBlendEnable = FALSE;
	for ( uint32 Index = 0; Index < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++Index )
	{
		const D3D12_RENDER_TARGET_BLEND_DESC DefaultRenderTargetBlendDesc =
		{
			FALSE,FALSE,
			D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
			D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
			D3D12_LOGIC_OP_NOOP,
			D3D12_COLOR_WRITE_ENABLE_ALL,
		};
		BlendState.RenderTarget[ Index ] = DefaultRenderTargetBlendDesc;
	}
	// Simple alpha blending
	BlendState.RenderTarget[ 0 ].BlendEnable = true;
	BlendState.RenderTarget[ 0 ].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	BlendState.RenderTarget[ 0 ].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	BlendState.RenderTarget[ 0 ].BlendOp = D3D12_BLEND_OP_ADD;
	BlendState.RenderTarget[ 0 ].SrcBlendAlpha = D3D12_BLEND_ONE;
	BlendState.RenderTarget[ 0 ].DestBlendAlpha = D3D12_BLEND_ZERO;
	BlendState.RenderTarget[ 0 ].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	BlendState.RenderTarget[ 0 ].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC PsoDesc{};
	PsoDesc.RasterizerState = RasterizerDesc;
	PsoDesc.BlendState = BlendState;
	PsoDesc.VS.BytecodeLength = VertexShader->GetBufferSize();
	PsoDesc.VS.pShaderBytecode = VertexShader->GetBufferPointer();
	PsoDesc.PS.BytecodeLength = PixelShader->GetBufferSize();
	PsoDesc.PS.pShaderBytecode = PixelShader->GetBufferPointer();
	PsoDesc.pRootSignature = RootSignature;
	PsoDesc.NumRenderTargets = 1;
	PsoDesc.RTVFormats[ 0 ] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	PsoDesc.DSVFormat = DXGI_FORMAT_UNKNOWN;
	PsoDesc.InputLayout.NumElements = std::extent<decltype( Layout )>::value;
	PsoDesc.InputLayout.pInputElementDescs = Layout;
	PsoDesc.SampleDesc.Count = 1;
	PsoDesc.DepthStencilState.DepthEnable = false;
	PsoDesc.DepthStencilState.StencilEnable = false;
	PsoDesc.SampleMask = 0xFFFFFFFF;
	PsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	Device->CreateGraphicsPipelineState( &PsoDesc, IID_PPV_ARGS( &PSO ) );

	return std::make_shared<CPipelineStateDX12>( PSO, RootSignature );
}

IVertexBufferContextRef CGraphicsInterfaceDX12::CreateVertexBuffer( void* Data, uint64 SizeInByte )
{
	return CreateBuffer< SVertexBufferContextDX12 >( Data, SizeInByte );
}

IIndexBufferContextRef CGraphicsInterfaceDX12::CreateIndexBuffer( void* Data, uint64 SizeInByte )
{
	return CreateBuffer< SIndexBufferContextDX12 >( Data, SizeInByte );
}

template<typename TBufferContext>
std::shared_ptr<TBufferContext> CGraphicsInterfaceDX12::CreateBuffer( void* Data, uint64 SizeInByte )
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

	CopyMemoryToBuffer( UploadBuffer, Data, SizeInByte );

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

	if ( Fence->GetCompletedValue() != 1 )
	{
		Fence->SetEventOnCompletion( 1, FenceEventHandle );
		WaitForSingleObject( FenceEventHandle, INFINITE );
	}

	return std::make_shared<TBufferContext>( OutBuffer );
}

void CGraphicsInterfaceDX12::CopyMemoryToBuffer( ID3D12Resource* Buffer, void* Data, uint64 SizeInByte )
{
	void* VirtualMem;
	Buffer->Map( 0, nullptr, &VirtualMem );
	::memcpy( VirtualMem, Data, SizeInByte );
	Buffer->Unmap( 0, nullptr );
}

void CGraphicsInterfaceDX12::SetWindowParams( const SGraphicsInitParamDX12& InParam )
{
	WindowViewport = { 0.0f, 0.0f,
		( float ) ( InParam.WindowRect.Right - InParam.WindowRect.Left ),
		( float ) ( InParam.WindowRect.Bottom - InParam.WindowRect.Top ),
		0.0f, 1.0f
	};

	WindowScissorRect = Convert::ToWindowsRect( InParam.WindowRect );
	WindowHandle = InParam.WindowHandle;
}

void CGraphicsInterfaceDX12::CreateDebugLayer()
{
	CHECK_HRESULT( D3D12GetDebugInterface( IID_PPV_ARGS( &DebugInterface ) ) );
	DebugInterface->EnableDebugLayer();
}

void CGraphicsInterfaceDX12::CreateDevice()
{
	CHECK_HRESULT( D3D12CreateDevice( nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS( &Device ) ) );
}

void CGraphicsInterfaceDX12::CreateDebugInfoQueue()
{
	CHECK_HRESULT( Device->QueryInterface( IID_PPV_ARGS( &DebugInfoQueue ) ) );
	DebugInfoQueue->SetBreakOnSeverity( D3D12_MESSAGE_SEVERITY_ERROR, true );
	DebugInfoQueue->SetBreakOnSeverity( D3D12_MESSAGE_SEVERITY_CORRUPTION, true );
	DebugInfoQueue->SetBreakOnSeverity( D3D12_MESSAGE_SEVERITY_WARNING, true );
	DebugInfoQueue->SetBreakOnID( D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE, true );
}

void CGraphicsInterfaceDX12::CreateCommandQueuesAndLists()
{
	auto _create_cmd_queue_and_lists = [&]( CDX12CommandQueueSet& CmdQueueSet, D3D12_COMMAND_LIST_TYPE CmdListType )
	{
		D3D12_COMMAND_QUEUE_DESC QueueDesc{};
		QueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		QueueDesc.Type = ( D3D12_COMMAND_LIST_TYPE ) CmdListType;

		CHECK_HRESULT( Device->CreateCommandQueue( &QueueDesc, IID_PPV_ARGS( &CmdQueueSet.CmdQueue ) ) );
		CHECK_HRESULT( Device->CreateCommandAllocator( QueueDesc.Type, IID_PPV_ARGS( &CmdQueueSet.Allocator ) ) );

		for ( int32 Index = 0; Index < COMMANDLIST_PER_QUEUE_COUNT; ++Index )
		{
			ID3D12GraphicsCommandList*& CmdList = CmdQueueSet.CmdLists[ Index ];

			CHECK_HRESULT( Device->CreateCommandList( 0, QueueDesc.Type, CmdQueueSet.Allocator, nullptr, IID_PPV_ARGS( &CmdList ) ) );
			CHECK_HRESULT( CmdList->Close() );
		}

		CHECK_HRESULT( Device->CreateFence( 0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS( &CmdQueueSet.Fence ) ) );

		CmdQueueSet.FenceEventHandle = CreateEvent( nullptr, FALSE, FALSE, nullptr );
		CmdQueueSet.FenceValue = 0;
		CmdQueueSet.CmdListIter = CmdQueueSet.CmdLists.begin();
	};

	D3D12_COMMAND_LIST_TYPE CmdListTypes[] = {
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		D3D12_COMMAND_LIST_TYPE_COMPUTE,
		D3D12_COMMAND_LIST_TYPE_COPY
	};

	for ( D3D12_COMMAND_LIST_TYPE CmdListType : CmdListTypes )
	{
		_create_cmd_queue_and_lists( CmdQueueSets[ CmdListType ], CmdListType );
	}
}

void CGraphicsInterfaceDX12::CreateSwapChain()
{
	CDX12CommandQueueSet& CmdQueueSet = CmdQueueSets[ D3D12_COMMAND_LIST_TYPE_DIRECT ];

	DXGI_SWAP_CHAIN_DESC SwapChainDesc{};
	SwapChainDesc.BufferCount = SWAPCHAIN_BUFFERCOUNT;
	SwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	SwapChainDesc.BufferDesc.Width = ( uint32 ) WindowViewport.Width;
	SwapChainDesc.BufferDesc.Height = ( uint32 ) WindowViewport.Height;
	SwapChainDesc.OutputWindow = WindowHandle;
	SwapChainDesc.SampleDesc.Count = 1;
	SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	SwapChainDesc.Windowed = true;

	IDXGIFactory6* DxgiFactory;
	CHECK_HRESULT( CreateDXGIFactory1( IID_PPV_ARGS( &DxgiFactory ) ) );
	CHECK_HRESULT( DxgiFactory->CreateSwapChain( CmdQueueSet.CmdQueue, &SwapChainDesc, &SwapChain ) );
}

void CGraphicsInterfaceDX12::CreateDescriptorHeap()
{
	D3D12_DESCRIPTOR_HEAP_DESC DescHeapDesc{};
	DescHeapDesc.NumDescriptors = SWAPCHAIN_BUFFERCOUNT;
	DescHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	DescHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	CHECK_HRESULT( Device->CreateDescriptorHeap( &DescHeapDesc, IID_PPV_ARGS( &DescHeap ) ) );
}

void CGraphicsInterfaceDX12::CreateRenderTargets()
{
	D3D12_CPU_DESCRIPTOR_HANDLE RenderTargetViewDescHandle = DescHeap->GetCPUDescriptorHandleForHeapStart();
	size_t RenderTargetViewDescSize = Device->GetDescriptorHandleIncrementSize( D3D12_DESCRIPTOR_HEAP_TYPE_RTV );

	for ( int32 BufferIndex = 0; BufferIndex < SWAPCHAIN_BUFFERCOUNT; ++BufferIndex )
	{
		SDX12SwapChainBufferContext& SwapChainBuffer = SwapChainBuffers[ BufferIndex ];
		SwapChainBuffer.BufferIndex = BufferIndex;

		D3D12_RENDER_TARGET_VIEW_DESC ViewDesc{};
		ViewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		ViewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		ViewDesc.Texture2D.MipSlice = 0;
		ViewDesc.Texture2D.PlaneSlice = 0;

		CHECK_HRESULT( SwapChain->GetBuffer( BufferIndex, IID_PPV_ARGS( &SwapChainBuffer.RenderTarget ) ) );

		Device->CreateRenderTargetView( SwapChainBuffer.RenderTarget, &ViewDesc, RenderTargetViewDescHandle );

		SwapChainBuffer.RenderTargetHandlePtr = RenderTargetViewDescHandle.ptr;
		RenderTargetViewDescHandle.ptr += RenderTargetViewDescSize;
	}

	SwapChainBufferIter = SwapChainBuffers.begin();
}