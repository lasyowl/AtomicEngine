#include "RB_DX12.h"

#include <Core/DebugUtil.h>
#include <RenderBackend/RBPipeline_DX12.h>
#include <RenderBackend/RBUtility_DX12.h>
#include <RenderBackend/RBUtility.h>

#include <d3d12.h>
#include <d3dcommon.h>
#include <dxgi.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <comdef.h>
#include <fstream>
#include <dxcapi.h>
#include <span>

#define CHECK_HRESULT( hr, msg ) \
    if(FAILED (hr)) {\
        AEMessageBox( msg );\
    }

////////////////////////////////
// Misc Functions
////////////////////////////////
static constexpr D3D12_VIEWPORT ToDX12Viewport( uint32 width, uint32 height )
{
    return D3D12_VIEWPORT{ 0, 0, (float)width, (float)height, 0, 1 };
}

static constexpr D3D12_RECT ToDX12Rect( uint32 width, uint32 height )
{
    return D3D12_RECT{ 0, 0, (int32)width, (int32)height };
}

static const char* ShaderTypeToString( EShaderType type )
{
    switch ( type )
    {
        case EShaderType::VertexShader:     return "vs_5_0";
        case EShaderType::PixelShader:      return "ps_5_0";
        case EShaderType::ComputeShader:    return "cs_5_0";
        //case EShaderType::RayTraceShader: return "lib_6_3";
    }

    return nullptr;
}

static D3D12_INPUT_ELEMENT_DESC TranslateInputDesc( const RBPipelineInputDesc& inputDesc )
{
    D3D12_INPUT_ELEMENT_DESC d3dInputDesc =
    {
        inputDesc.semanticName.c_str(),
        0,
        RBUtil::TranslateResourceFormat( inputDesc.format ),
        inputDesc.inputSlot,
        inputDesc.byteOffset,
        inputDesc.inputClass == RBInputClass_PerInstance ? D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA : D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
        0
    };

    return d3dInputDesc;
}

static std::vector<D3D_SHADER_MACRO> TranslateShaderMacros( const std::vector<RBShaderMacro>& inMacros )
{
    std::vector<D3D_SHADER_MACRO> macros;
    macros.resize( inMacros.size() + 1 );
    for ( uint32 index = 0; index < inMacros.size(); ++index )
    {
        macros[ index ].Name = inMacros[ index ].name.c_str();
        macros[ index ].Definition = inMacros[ index ].value.c_str();
    }
    macros[ inMacros.size() ] = {};

    return macros;
}

static void WaitForFence( CommandQueueContext& cmdQueueCtx )
{
    ID3D12Fence* fence = cmdQueueCtx.fence;
    HANDLE fenceEventHandle = cmdQueueCtx.fenceEventHandle;
    uint64& fenceValue = cmdQueueCtx.fenceValue;

    CHECK_HRESULT( cmdQueueCtx.cmdQueue->Signal( fence, ++fenceValue ), L"Failed to signal command queue." );

    uint64 completeValue = fence->GetCompletedValue();
    if ( completeValue != fenceValue )
    {
        fence->SetEventOnCompletion( fenceValue, fenceEventHandle );
        WaitForSingleObject( fenceEventHandle, INFINITE );
    }
}

static constexpr D3D12_HEAP_PROPERTIES HeapProperties( D3D12_HEAP_TYPE heapType )
{
    D3D12_HEAP_PROPERTIES heapProp{};
    heapProp.Type = heapType;
    heapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    heapProp.CreationNodeMask = 1;
    heapProp.VisibleNodeMask = 1;

    return heapProp;
}

static void CopyMemoryToBuffer( ID3D12Resource* buffer, const void* data, uint64 size )
{
    void* virtualMem;
    CHECK_HRESULT( buffer->Map( 0, nullptr, &virtualMem ), L"Failed to map buffer." );
    ::memcpy( virtualMem, data, size );
    buffer->Unmap( 0, nullptr );
}

static void create_resource( ID3D12Device* device, RBResource_DX12& resource, const RBResourceDesc& desc )
{
    // todo : set clear value for depthstencil and others..

    D3D12_RESOURCE_DESC translatedDesc = RBUtil::TranslateResourceDesc( desc );
    D3D12_HEAP_PROPERTIES defaultHeapProp = HeapProperties( D3D12_HEAP_TYPE_DEFAULT );

    ID3D12Resource* native;

    if ( desc.clearValue.type == ERBResourceClearValueType::None )
    {
        CHECK_HRESULT( device->CreateCommittedResource( &defaultHeapProp,
                                                         D3D12_HEAP_FLAG_NONE,
                                                         &translatedDesc,
                                                         RBUtil::TranslateResourceState( desc.initialState ),
                                                         nullptr,
                                                         IID_PPV_ARGS( &native ) ),
                       L"Failed to create output buffer." );
    }
    else
    {
        D3D12_CLEAR_VALUE clearValue = RBUtil::TranslateResourceClearValue( desc );

        CHECK_HRESULT( device->CreateCommittedResource( &defaultHeapProp,
                                                         D3D12_HEAP_FLAG_NONE,
                                                         &translatedDesc,
                                                         RBUtil::TranslateResourceState( desc.initialState ),
                                                         &clearValue,
                                                         IID_PPV_ARGS( &native ) ),
                       L"Failed to create output buffer." );
    }

    if ( !desc.name.empty() )
    {
        std::wstring name;
        name.assign( desc.name.begin(), desc.name.end() );

        native->SetName( name.c_str() );
    }

    resource.state = desc.initialState;
    resource.native = native;
}

static void create_render_target_view( ID3D12Device* device, RBDescriptorHeapAllocator_DX12& heap, RBRenderTargetView_DX12& rtv, ID3D12Resource* resource, const RBRenderTargetViewDesc& rtvDesc )
{
    rtv.handle = rtvDesc.bStatic ? heap.AllocateStatic( RBResourceViewType_RTV ) : heap.AllocateDynamic( RBResourceViewType_RTV );
    D3D12_RENDER_TARGET_VIEW_DESC translatedDesc = RBUtil::TranslateRTVDesc( rtvDesc );
    device->CreateRenderTargetView( resource, &translatedDesc, rtv.handle.cpu );
}

static void create_depth_stencil_view( ID3D12Device* device, RBDescriptorHeapAllocator_DX12& heap, RBDepthStencilView_DX12& dsv, ID3D12Resource* resource, const RBDepthStencilViewDesc& dsvDesc )
{
    dsv.handle = dsvDesc.bStatic ? heap.AllocateStatic( RBResourceViewType_DSV ) : heap.AllocateDynamic( RBResourceViewType_DSV );
    D3D12_DEPTH_STENCIL_VIEW_DESC translatedDesc = RBUtil::TranslateDSVDesc( dsvDesc );
    device->CreateDepthStencilView( resource, &translatedDesc, dsv.handle.cpu );
}

////////////////////////////////
// RBResourceAllocator_DX12
////////////////////////////////
void RBResourceAllocator_DX12::Initialize()
{
    constexpr uint32 NUM_RESOURCES = 10000;
    for ( uint32 iter = 0; iter < NUM_RESOURCES; ++iter )
    {
        _freeResourceIDs.push( iter );
    }
}

uint32 RBResourceAllocator_DX12::AllocateUniqueResourceID( ID3D12Resource* resource )
{
    uint32 resourceID = _freeResourceIDs.front();
    _freeResourceIDs.pop();

    assert( !_resourceCache.contains( resourceID ) );
    _resourceCache[ resourceID ] = resource;

    return resourceID;
}

void RBResourceAllocator_DX12::ReleaseUniqueResourceID( uint32 resourceID )
{
    _freeResourceIDs.push( resourceID );
}

uint32 RBResourceAllocator_DX12::CreateResource( ID3D12Device* device, const RBResourceDesc& desc )
{
    // todo : set clear value for depthstencil and others..

    D3D12_RESOURCE_DESC translatedDesc = RBUtil::TranslateResourceDesc( desc );
    D3D12_HEAP_PROPERTIES defaultHeapProp = HeapProperties( D3D12_HEAP_TYPE_DEFAULT );

    ID3D12Resource* resource;
    CHECK_HRESULT( device->CreateCommittedResource( &defaultHeapProp,
                                                    D3D12_HEAP_FLAG_NONE,
                                                    &translatedDesc,
                                                    RBUtil::TranslateResourceState( desc.initialState ),
                                                    nullptr,
                                                    IID_PPV_ARGS( &resource ) ),
                   L"Failed to create output buffer." );

    return AllocateUniqueResourceID( resource );
}

ID3D12Resource* RBResourceAllocator_DX12::GetResource( uint32 resourceID )
{
    assert( _resourceCache.contains( resourceID ) );

    return _resourceCache[ resourceID ];
}

///////////////////////////////////////
// RB_DX12
///////////////////////////////////////
RB_DX12::RB_DX12( const HWND hWnd, const IVec2& windowSize )
    : _device( nullptr )
    , _swapChain( nullptr )
    , _swapChainIndex( 0 )
    , _debugInterface( nullptr )
    , _debugInterfaceEx( nullptr )
    , _hWnd( hWnd )
{
    SetWindowSize( windowSize );
}

RB_DX12::~RB_DX12()
{
    _heap.Release();

#define CHECKED_RELEASE(ref)\
    if(ref) ref->Release();

    CHECKED_RELEASE( _debugInterfaceEx );
    CHECKED_RELEASE( _debugInterface );
    CHECKED_RELEASE( _swapChain );
    CHECKED_RELEASE( _device );

#undef CHECKED_RELEASE
}

void RB_DX12::Startup()
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

        for ( D3D12_COMMAND_LIST_TYPE type : cmdListTypes )
        {
            CommandQueueContext& cmdQueueCtx = _cmdQueueCtx[ type ];

            D3D12_COMMAND_QUEUE_DESC queueDesc{};
            queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
            queueDesc.Type = type;

            CHECK_HRESULT( _device->CreateCommandQueue( &queueDesc, IID_PPV_ARGS( &cmdQueueCtx.cmdQueue ) ),
                           L"Failed to create command queue." );
            CHECK_HRESULT( _device->CreateCommandAllocator( queueDesc.Type, IID_PPV_ARGS( &cmdQueueCtx.allocator ) ),
                           L"Failed to create command allocator." );

            for ( int32 Index = 0; Index < kCmdListPerQueueCount; ++Index )
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

    _heap.Initialize( _device );
    _resource.Initialize();

    /* Create swapchain */
    {
        DXGI_SWAP_CHAIN_DESC swapChainDesc{};
        swapChainDesc.BufferCount = kSwapChainCount;
        swapChainDesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.BufferDesc.Width = GetWindowSize().x;
        swapChainDesc.BufferDesc.Height = GetWindowSize().y;
        swapChainDesc.OutputWindow = _hWnd;
        swapChainDesc.SampleDesc.Count = 1;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        swapChainDesc.Windowed = true;

        IDXGIFactory6* dxgiFactory;
        CHECK_HRESULT( CreateDXGIFactory1( IID_PPV_ARGS( &dxgiFactory ) ),
                       L"Failed to create dxgi factory." );

        CommandQueueContext& cmdQueueCtx = _cmdQueueCtx[ D3D12_COMMAND_LIST_TYPE_DIRECT ];
        CHECK_HRESULT( dxgiFactory->CreateSwapChain( cmdQueueCtx.cmdQueue, &swapChainDesc, &_swapChain ),
                       L"Failed to create swapchain." );

        const RBRenderTargetViewDesc rtvDesc
        {
            .bStatic = true,
            .format = ERBResourceFormat::B8G8R8A8,
            .dimension = ERBResourceDimension::Texture2D
        };
        const RBDepthStencilViewDesc dsvDesc
        {
            .bStatic = true,
            .format = ERBResourceFormat::D32_Float,
            .dimension = ERBResourceDimension::Texture2D,
            .flag = RBDepthStencilViewFlag_None
        };

        RBResource_DX12 depthResource;
        RBDepthStencilView_DX12 dsv;
        create_resource( _device, depthResource, RBUtil::GetDepthStencilResourceDesc( GetWindowSize() ) );
        create_depth_stencil_view( _device, _heap, dsv, depthResource.native, dsvDesc );

        for ( int32 index = 0; index < kSwapChainCount; ++index )
        {
            RBSwapChain_DX12& swapChainResource = _swapChainResources[ index ];
            CHECK_HRESULT( _swapChain->GetBuffer( index, IID_PPV_ARGS( &swapChainResource.color.native ) ), L"Failed to get swapchain buffer." );

            create_render_target_view( _device, _heap, swapChainResource.rtv, swapChainResource.color.native, rtvDesc );
            swapChainResource.color.state = RBResourceState_RenderTarget;
            swapChainResource.depth = depthResource;
            swapChainResource.dsv = dsv;
        }
    }
}

void RB_DX12::BeginFrame()
{
    const RBSwapChain_DX12& swapChain = _swapChainResources[ _swapChainIndex ];

    CommandQueueContext& cmdQueueCtx = _cmdQueueCtx[ D3D12_COMMAND_LIST_TYPE_DIRECT ];
    ID3D12CommandAllocator* cmdAllocator = cmdQueueCtx.allocator;
    ID3D12GraphicsCommandList* cmdList = *cmdQueueCtx.iCmdList;

    CHECK_HRESULT( cmdAllocator->Reset(), L"Failed to reset command allocator." );
    CHECK_HRESULT( cmdList->Reset( cmdAllocator, nullptr ), L"Failed to reset command list." );

    TransitionResource( cmdList, swapChain.color.native, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET );

    static float clearColor[] = { 0.242f, 0.242f, 0.442f, 1 };

    cmdList->ClearRenderTargetView( swapChain.rtv.handle.cpu, clearColor, 0, nullptr );
    cmdList->ClearDepthStencilView( swapChain.dsv.handle.cpu, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr );
}

void RB_DX12::EndFrame()
{
    const RBSwapChain_DX12& swapChain = _swapChainResources[ _swapChainIndex ];

    CommandQueueContext& cmdQueueCtx = _cmdQueueCtx[ D3D12_COMMAND_LIST_TYPE_DIRECT ];
    ID3D12GraphicsCommandList* cmdList = *cmdQueueCtx.iCmdList;

    TransitionResource( cmdList, swapChain.color.native, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT );

    CHECK_HRESULT( cmdList->Close(), L"Failed to close command list." );

    ID3D12CommandList* cmdListInterface = cmdList;
    cmdQueueCtx.cmdQueue->ExecuteCommandLists( 1, &cmdListInterface );

    CHECK_HRESULT( _swapChain->Present( 1, 0 ), L"Failed to present swapchain." );

    WaitForFence( cmdQueueCtx );

    _swapChainIndex = ( _swapChainIndex + 1 ) % kSwapChainCount;

    if ( ++cmdQueueCtx.iCmdList == cmdQueueCtx.cmdLists.end() )
    {
        cmdQueueCtx.iCmdList = cmdQueueCtx.cmdLists.begin();
    }

    _heap.ClearDynamic();
}

void RB_DX12::ClearRenderTarget( const IRBResource& inResource, const IRBRenderTargetView& inRTV )
{
    const RBResource_DX12& resource = static_cast<const RBResource_DX12&>( inResource );
    const RBRenderTargetView_DX12& rtv = static_cast<const RBRenderTargetView_DX12&>( inRTV );

    CommandQueueContext& cmdQueueCtx = _cmdQueueCtx[ D3D12_COMMAND_LIST_TYPE_DIRECT ];
    ID3D12GraphicsCommandList* cmdList = *cmdQueueCtx.iCmdList;

    if ( false/*@TODO: Has uav*/ )
    {
        TransitionResource( cmdList, resource.native, RBUtil::TranslateResourceState( inResource.state ), D3D12_RESOURCE_STATE_UNORDERED_ACCESS );

        uint32 clearValue[ 4 ]{};
        cmdList->ClearUnorderedAccessViewUint( rtv.handle.gpu, rtv.handle.cpu, resource.native, clearValue, 0, nullptr );

        TransitionResource( cmdList, resource.native, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, RBUtil::TranslateResourceState( inResource.state ) );
    }
    else
    {
        float clearValue[ 4 ]{};
        D3D12_RECT rect = ToDX12Rect( GetWindowSize().x, GetWindowSize().y );
        cmdList->ClearRenderTargetView( rtv.handle.cpu, clearValue, 1, &rect );
    }
}

void RB_DX12::SetGraphicsPipelineState( const IRBPipeline& inPipeline )
{
    const RBPipeline_DX12& pipeline = static_cast<const RBPipeline_DX12&>( inPipeline );

    CommandQueueContext& cmdQueueCtx = _cmdQueueCtx[ D3D12_COMMAND_LIST_TYPE_DIRECT ];
    ID3D12GraphicsCommandList* cmdList = *cmdQueueCtx.iCmdList;

    cmdList->SetPipelineState( pipeline.pipelineState );
    cmdList->SetGraphicsRootSignature( pipeline.rootSignature );
    cmdList->IASetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

    D3D12_VIEWPORT viewport = ToDX12Viewport( GetWindowSize().x, GetWindowSize().y );
    D3D12_RECT scissorRect = ToDX12Rect( GetWindowSize().x, GetWindowSize().y );
    cmdList->RSSetViewports( 1, &viewport );
    cmdList->RSSetScissorRects( 1, &scissorRect );
}

void RB_DX12::SetInputAssembly( const IRBVertexBufferView& inVbv )
{
    const RBVertexBufferView_DX12& vbv = static_cast<const RBVertexBufferView_DX12&>( inVbv );

    CommandQueueContext& cmdQueueCtx = _cmdQueueCtx[ D3D12_COMMAND_LIST_TYPE_DIRECT ];
    ID3D12GraphicsCommandList* cmdList = *cmdQueueCtx.iCmdList;

    cmdList->IASetVertexBuffers( 0, 1, &vbv.native );
}

void RB_DX12::SetInputAssembly( const IRBVertexBufferView& inVbv, const IRBIndexBufferView& inIbv )
{
    const RBVertexBufferView_DX12& vbv = static_cast<const RBVertexBufferView_DX12&>( inVbv );
    const RBIndexBufferView_DX12& ibv = static_cast<const RBIndexBufferView_DX12&>( inIbv );

    CommandQueueContext& cmdQueueCtx = _cmdQueueCtx[ D3D12_COMMAND_LIST_TYPE_DIRECT ];
    ID3D12GraphicsCommandList* cmdList = *cmdQueueCtx.iCmdList;

    cmdList->IASetVertexBuffers( 0, 1, &vbv.native );
    cmdList->IASetIndexBuffer( &ibv.native );
}

void RB_DX12::DrawInstanced( uint32 numVertices, uint32 numInstances, uint32 vertexOffset, uint32 instanceOffset )
{
    CommandQueueContext& cmdQueueCtx = _cmdQueueCtx[ D3D12_COMMAND_LIST_TYPE_DIRECT ];
    ID3D12GraphicsCommandList* cmdList = *cmdQueueCtx.iCmdList;
    cmdList->DrawInstanced( numVertices, numInstances, vertexOffset, instanceOffset );
}

void RB_DX12::DrawIndexedInstanced( uint32 numIndices, uint32 numInstances, uint32 indexOffset, uint32 vertexOffset, uint32 instanceOffset )
{
    CommandQueueContext& cmdQueueCtx = _cmdQueueCtx[ D3D12_COMMAND_LIST_TYPE_DIRECT ];
    ID3D12GraphicsCommandList* cmdList = *cmdQueueCtx.iCmdList;
    cmdList->DrawIndexedInstanced( numIndices, numInstances, indexOffset, vertexOffset, instanceOffset );
}

void RB_DX12::ExecuteCommandList()
{
    CommandQueueContext& cmdQueueCtx = _cmdQueueCtx[ D3D12_COMMAND_LIST_TYPE_DIRECT ];
    ID3D12CommandAllocator* cmdAllocator = cmdQueueCtx.allocator;
    ID3D12GraphicsCommandList* cmdList = *cmdQueueCtx.iCmdList;

    CHECK_HRESULT( cmdList->Close(), L"Failed to close command list." );

    ID3D12CommandList* cmdListInterface = cmdList;
    cmdQueueCtx.cmdQueue->ExecuteCommandLists( 1, &cmdListInterface );

    WaitForFence( cmdQueueCtx );

    CHECK_HRESULT( cmdAllocator->Reset(), L"Failed to reset allocator." );
    CHECK_HRESULT( cmdList->Reset( cmdAllocator, nullptr ), L"Failed to reset command list." );
}

IRBResource* RB_DX12::GetSwapChainColorResource()
{
    return &_swapChainResources[ _swapChainIndex ].color;
}

IRBResource* RB_DX12::GetSwapChainDepthResource()
{
    return &_swapChainResources[ _swapChainIndex ].depth;
}

IRBRenderTargetView* RB_DX12::GetSwapChainRenderTargetView()
{
    return &_swapChainResources[ _swapChainIndex ].rtv;
}

IRBDepthStencilView* RB_DX12::GetSwapChainDepthStencilView()
{
    return &_swapChainResources[ _swapChainIndex ].dsv;
}

static IDxcBlob* create_shader2( const RBShaderDesc& shaderDesc )
{
    // Initialize the DXC compiler and library
    IDxcCompiler* pCompiler;
    IDxcLibrary* pLibrary;
    DxcCreateInstance( CLSID_DxcCompiler, IID_PPV_ARGS( &pCompiler ) );
    DxcCreateInstance( CLSID_DxcLibrary, IID_PPV_ARGS( &pLibrary ) );

    std::ifstream shaderFile( shaderDesc.file, std::ios_base::in );
    std::string parsedShader = std::string( std::istreambuf_iterator<char>( shaderFile ),
                                            std::istreambuf_iterator<char>() );

    // Prepare HLSL shader code as a string or load from a file
    IDxcBlobEncoding* pSource;
    pLibrary->CreateBlobWithEncodingFromPinned( parsedShader.data(), parsedShader.size(), CP_UTF8, &pSource );

    // Compile the shader
    IDxcOperationResult* pResult;
    pCompiler->Compile(
        pSource, // pSourceBlob
        nullptr,//L"RayTracingTest.hlsl", // pSourceName
        nullptr,//L"RayGeneration", // pEntryPoint
        L"lib_6_3", // pTargetProfile
        nullptr, 0, // pArguments, argCount
        nullptr, 0, // pDefines, defineCount
        nullptr, // pIncludeHandler
        &pResult );

    // Check for errors and retrieve the compiled shader bytecode
    HRESULT hr;
    pResult->GetStatus( &hr );
    if ( SUCCEEDED( hr ) )
    {
        IDxcBlob* pShader;
        pResult->GetResult( &pShader );
        // Use the compiled shader (pShader) in your application
        return pShader;
    }
    else
    {
        // Handle compilation errors
        IDxcBlobEncoding* pError;
        pResult->GetErrorBuffer( &pError );
        // Output the compilation errors
        std::string errorMsg = (char*)pError->GetBufferPointer();
        std::wstring wErrorMsg;
        wErrorMsg.append( errorMsg.begin(), errorMsg.end() );
        AEMessageBox( wErrorMsg );
        return nullptr;
    }
}

static ID3DBlob* create_shader( const RBShaderDesc& shaderDesc )
{
    std::vector<D3D_SHADER_MACRO> macros = TranslateShaderMacros( shaderDesc.macros );

    std::ifstream shaderFile( shaderDesc.file, std::ios_base::in );
    std::string parsedShader = std::string( std::istreambuf_iterator<char>( shaderFile ),
                                            std::istreambuf_iterator<char>() );

    ID3DBlob* compiledShader;
    ID3DBlob* errorBlob;
    CHECK_HRESULT( D3DCompile( parsedShader.c_str(),
                               parsedShader.size(),
                               shaderDesc.file.c_str(),
                               macros.data(),
                               D3D_COMPILE_STANDARD_FILE_INCLUDE,
                               shaderDesc.entry.c_str(),
                               ShaderTypeToString( shaderDesc.type ),
                               0,
                               0,
                               &compiledShader,
                               &errorBlob ),
                   L"Shader compilation failed." );
    if ( errorBlob )
    {
        std::string errorMsg = (char*)errorBlob->GetBufferPointer();
        std::wstring wErrorMsg;
        wErrorMsg.assign( errorMsg.begin(), errorMsg.end() );
        AEMessageBox( wErrorMsg );
    }

    return compiledShader;
}

IRBShaderPtr RB_DX12::CreateShader( const RBShaderDesc& shaderDesc )
{
    std::shared_ptr<RBShader_DX12> shader( new RBShader_DX12() );
    shader->native = create_shader( shaderDesc );

    return shader;
}

static ID3D12RootSignature* create_graphics_root_signature( ID3D12Device* device, const RBPipelineStateDesc& pipelineDesc )
{
    ID3D12RootSignature* rootSignature;

    std::vector<D3D12_ROOT_PARAMETER> rootParams;
    std::vector<D3D12_DESCRIPTOR_RANGE> descriptorRanges;

    // @TODO: Remove duplicates, VS and PS
    for ( const RBShaderParameter& shaderParam : pipelineDesc.vertexShaderDesc->parameters )
    {
        D3D12_ROOT_PARAMETER& param = rootParams.emplace_back();
        param.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

        switch ( shaderParam.type )
        {
            case EShaderParameterType::Constant:
            {
                param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
                param.Constants.Num32BitValues = shaderParam.constantCount;
                param.Constants.ShaderRegister = shaderParam.registerIndex;
                param.Constants.RegisterSpace = shaderParam.spaceIndex;
                break;
            }
            case EShaderParameterType::ConstantBuffer:
            {
                param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
                param.Descriptor.ShaderRegister = shaderParam.registerIndex;
                param.Descriptor.RegisterSpace = shaderParam.spaceIndex;
                break;
            }
            case EShaderParameterType::ShaderResource:
            {
                param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
                param.Descriptor.ShaderRegister = shaderParam.registerIndex;
                param.Descriptor.RegisterSpace = shaderParam.spaceIndex;
                break;
            }
            case EShaderParameterType::UnorderedAccess:
            {
                param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_UAV;
                param.Descriptor.ShaderRegister = shaderParam.registerIndex;
                param.Descriptor.RegisterSpace = shaderParam.spaceIndex;
                break;
            }
            case EShaderParameterType::DescriptorTable:
            {
                D3D12_DESCRIPTOR_RANGE& range = descriptorRanges.emplace_back();
                range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
                range.NumDescriptors = shaderParam.resourceCount;
                range.BaseShaderRegister = shaderParam.registerIndex;
                range.RegisterSpace = shaderParam.spaceIndex;

                param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
                param.DescriptorTable.NumDescriptorRanges = 1;
                param.DescriptorTable.pDescriptorRanges = &range;
                break;
            }
        }
    }

    for ( const RBShaderParameter& shaderParam : pipelineDesc.pixelShaderDesc->parameters )
    {
        D3D12_ROOT_PARAMETER& param = rootParams.emplace_back();
        param.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

        switch ( shaderParam.type )
        {
            case EShaderParameterType::Constant:
            {
                param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
                param.Constants.Num32BitValues = shaderParam.constantCount;
                param.Constants.ShaderRegister = shaderParam.registerIndex;
                param.Constants.RegisterSpace = shaderParam.spaceIndex;
                break;
            }
            case EShaderParameterType::ConstantBuffer:
            {
                param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
                param.Descriptor.ShaderRegister = shaderParam.registerIndex;
                param.Descriptor.RegisterSpace = shaderParam.spaceIndex;
                break;
            }
            case EShaderParameterType::ShaderResource:
            {
                param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
                param.Descriptor.ShaderRegister = shaderParam.registerIndex;
                param.Descriptor.RegisterSpace = shaderParam.spaceIndex;
                break;
            }
            case EShaderParameterType::UnorderedAccess:
            {
                param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_UAV;
                param.Descriptor.ShaderRegister = shaderParam.registerIndex;
                param.Descriptor.RegisterSpace = shaderParam.spaceIndex;
                break;
            }
            case EShaderParameterType::DescriptorTable:
            {
                D3D12_DESCRIPTOR_RANGE& range = descriptorRanges.emplace_back();
                range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
                range.NumDescriptors = shaderParam.resourceCount;
                range.BaseShaderRegister = shaderParam.registerIndex;
                range.RegisterSpace = shaderParam.spaceIndex;

                param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
                param.DescriptorTable.NumDescriptorRanges = 1;
                param.DescriptorTable.pDescriptorRanges = &range;
                break;
            }
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
    if ( errorBlob )
    {
        std::string errorMsg = (char*)errorBlob->GetBufferPointer();
        std::wstring wErrorMsg;
        wErrorMsg.assign( errorMsg.begin(), errorMsg.end() );
        AEMessageBox( wErrorMsg );
    }

    CHECK_HRESULT( device->CreateRootSignature( 0, rootBlob->GetBufferPointer(), rootBlob->GetBufferSize(), IID_PPV_ARGS( &rootSignature ) ),
                   L"Failed to create root signature" );

    return rootSignature;
}

static ID3D12PipelineState* create_graphics_pipeline_state( ID3D12Device* device, const RBPipelineStateDesc& pipelineDesc, ID3D12RootSignature* rootSignature, const IRBShader* inVertexShader, const IRBShader* inPixelShader )
{
    const RBShader_DX12* vertexShader = static_cast<const RBShader_DX12*>( inVertexShader );
    const RBShader_DX12* pixelShader = static_cast<const RBShader_DX12*>( inPixelShader );

    ID3D12PipelineState* pso;

    std::vector<D3D12_INPUT_ELEMENT_DESC> layout;
    layout.resize( pipelineDesc.inputDesc.size() );
    for ( uint32 index = 0; index < layout.size(); ++index )
    {
        layout[ index ] = TranslateInputDesc( pipelineDesc.inputDesc[ index ] );
    }

    D3D12_RASTERIZER_DESC rasterizerDesc{};
    rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
    rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
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
    for ( uint32 Index = 0; Index < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++Index )
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
    for ( uint32 index = 0; index < pipelineDesc.rtvFormats.size(); ++index )
    {
        blendState.RenderTarget[ index ].BlendEnable = pipelineDesc.enableBlend;
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
    psoDesc.VS.BytecodeLength = vertexShader->native->GetBufferSize();
    psoDesc.VS.pShaderBytecode = vertexShader->native->GetBufferPointer();
    psoDesc.PS.BytecodeLength = pixelShader->native->GetBufferSize();
    psoDesc.PS.pShaderBytecode = pixelShader->native->GetBufferPointer();
    psoDesc.pRootSignature = rootSignature;
    psoDesc.NumRenderTargets = pipelineDesc.rtvFormats.size();
    for ( uint32 index = 0; index < pipelineDesc.rtvFormats.size(); ++index )
    {
        psoDesc.RTVFormats[ index ] = RBUtil::TranslateResourceFormat( pipelineDesc.rtvFormats[ index ] );
    }
    psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    psoDesc.InputLayout.NumElements = layout.size();
    psoDesc.InputLayout.pInputElementDescs = layout.data();
    psoDesc.SampleDesc.Count = 1;
    psoDesc.DepthStencilState.DepthEnable = pipelineDesc.enableDepth;
    psoDesc.DepthStencilState.DepthWriteMask = pipelineDesc.enableDepth ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
    psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
    psoDesc.DepthStencilState.StencilEnable = false;
    psoDesc.SampleMask = 0xFFFFFFFF;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

    device->CreateGraphicsPipelineState( &psoDesc, IID_PPV_ARGS( &pso ) );

    return pso;
}

IRBPipelineRef RB_DX12::CreateGraphicsPipelineState( const RBPipelineStateDesc& pipelineDesc )
{
    assert( pipelineDesc.pipelineType == PipelineType_Graphics );

    std::shared_ptr<RBPipeline_DX12> pipelineState = std::make_shared<RBPipeline_DX12>();

    pipelineState->rootSignature = create_graphics_root_signature( _device, pipelineDesc );
    pipelineState->pipelineState = create_graphics_pipeline_state( _device, pipelineDesc, pipelineState->rootSignature, pipelineDesc.vertexShader.get(), pipelineDesc.pixelShader.get() );

    return pipelineState;
}

IRBResourcePtr RB_DX12::CreateResource( const RBResourceDesc& desc )
{
    // todo : set clear value for depthstencil and others..

    D3D12_RESOURCE_DESC translatedDesc = RBUtil::TranslateResourceDesc( desc );
    D3D12_HEAP_PROPERTIES defaultHeapProp = HeapProperties( D3D12_HEAP_TYPE_DEFAULT );

    ID3D12Resource* native;

    if ( desc.clearValue.type == ERBResourceClearValueType::None )
    {
        CHECK_HRESULT( _device->CreateCommittedResource( &defaultHeapProp,
                                                         D3D12_HEAP_FLAG_NONE,
                                                         &translatedDesc,
                                                         RBUtil::TranslateResourceState( desc.initialState ),
                                                         nullptr,
                                                         IID_PPV_ARGS( &native ) ),
                       L"Failed to create output buffer." );
    }
    else
    {
        D3D12_CLEAR_VALUE clearValue = RBUtil::TranslateResourceClearValue( desc );

        CHECK_HRESULT( _device->CreateCommittedResource( &defaultHeapProp,
                                                         D3D12_HEAP_FLAG_NONE,
                                                         &translatedDesc,
                                                         RBUtil::TranslateResourceState( desc.initialState ),
                                                         &clearValue,
                                                         IID_PPV_ARGS( &native ) ),
                       L"Failed to create output buffer." );
    }

    if ( !desc.name.empty() )
    {
        std::wstring name;
        name.assign( desc.name.begin(), desc.name.end() );

        native->SetName( name.c_str() );
    }

    auto resource = std::make_shared<RBResource_DX12>();
    resource->state = desc.initialState;
    resource->native = native;

    return resource;
}

IRBResourcePtr RB_DX12::CreateResource( const RBResourceDesc& desc, const void* data, uint32 sizeInBytes )
{
    ID3D12Resource* native = CreateResource_Inner( desc, data, sizeInBytes );

    auto resource = std::make_shared<RBResource_DX12>();
    resource->state = desc.initialState;
    resource->native = native;

    return resource;
}

IRBRenderTargetViewPtr RB_DX12::CreateRenderTargetView( const IRBResource& inResource, const RBRenderTargetViewDesc& rtvDesc )
{
    const RBResource_DX12& resource = static_cast<const RBResource_DX12&>( inResource );

    auto rtv = std::make_shared<RBRenderTargetView_DX12>();

    create_render_target_view( _device, _heap, *rtv, resource.native, rtvDesc );

    return rtv;
}

IRBDepthStencilViewRef RB_DX12::CreateDepthStencilView( const IRBResource& inResource, const RBDepthStencilViewDesc& dsvDesc )
{
    const RBResource_DX12& resource = static_cast<const RBResource_DX12&>( inResource );

    auto dsv = std::make_shared<RBDepthStencilView_DX12>();

    create_depth_stencil_view( _device, _heap, *dsv, resource.native, dsvDesc );

    return dsv;
}

IRBConstantBufferViewPtr RB_DX12::CreateConstantBufferView( const IRBResource& inResource, const RBConstantBufferViewDesc& cbvDesc )
{
    const RBResource_DX12& resource = static_cast<const RBResource_DX12&>( inResource );

    //@TODO: refactoring needed
    const D3D12_RESOURCE_DESC desc = resource.native->GetDesc();
    const bool bUseVirtualAddress = desc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER;
    //

    std::shared_ptr<RBConstantBufferView_DX12> cbv = std::make_shared<RBConstantBufferView_DX12>();
    cbv->handle = cbvDesc.bStatic ? _heap.AllocateStatic( RBResourceViewType_CBV_SRV_UAV_SHADERVISIBLE ) : _heap.AllocateDynamic( RBResourceViewType_CBV_SRV_UAV_SHADERVISIBLE );
    cbv->gpuAddress = bUseVirtualAddress ? resource.native->GetGPUVirtualAddress() : 0;

    D3D12_CONSTANT_BUFFER_VIEW_DESC translatedDesc = RBUtil::TranslateCBVDesc( inResource, cbvDesc );
    _device->CreateConstantBufferView( &translatedDesc, cbv->handle.cpu );

    return cbv;
}

IRBShaderResourceViewPtr RB_DX12::CreateShaderResourceView( const IRBResource& inResource, const RBShaderResourceViewDesc& srvDesc )
{
    const RBResource_DX12& resource = static_cast<const RBResource_DX12&>( inResource );

    //@TODO: refactoring needed
    const D3D12_RESOURCE_DESC desc = resource.native->GetDesc();
    const bool bUseVirtualAddress = desc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER;
    //

    std::shared_ptr<RBShaderResourceView_DX12> srv = std::make_shared<RBShaderResourceView_DX12>();
    srv->handle = srvDesc.bStatic ? _heap.AllocateStatic( RBResourceViewType_CBV_SRV_UAV_SHADERHIDDEN ) : _heap.AllocateDynamic( RBResourceViewType_CBV_SRV_UAV_SHADERHIDDEN );
    srv->gpuAddress = bUseVirtualAddress ? resource.native->GetGPUVirtualAddress() : 0;

    D3D12_SHADER_RESOURCE_VIEW_DESC translatedDesc = RBUtil::TranslateSRVDesc( inResource, srvDesc );
    _device->CreateShaderResourceView( resource.native, &translatedDesc, srv->handle.cpu );

    return srv;
}

IRBUnorderedAccessViewPtr RB_DX12::CreateUnorderedAccessView( const IRBResource& inResource, const RBUnorderedAccessViewDesc& uavDesc, const bool bShadeRBdden )
{
    const RBResource_DX12& resource = static_cast<const RBResource_DX12&>( inResource );

    //@TODO: refactoring needed
    const D3D12_RESOURCE_DESC desc = resource.native->GetDesc();
    const bool bUseVirtualAddress = desc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER;
    const ERBResourceViewType resourceViewType = bShadeRBdden ? RBResourceViewType_CBV_SRV_UAV_SHADERHIDDEN : RBResourceViewType_CBV_SRV_UAV_SHADERVISIBLE;
    //

    std::shared_ptr<RBUnorderedAccessView_DX12> uav = std::make_shared<RBUnorderedAccessView_DX12>();
    uav->handle = uavDesc.bStatic ? _heap.AllocateStatic( resourceViewType ) : _heap.AllocateDynamic( resourceViewType );
    uav->gpuAddress = bUseVirtualAddress ? resource.native->GetGPUVirtualAddress() : 0;

    D3D12_UNORDERED_ACCESS_VIEW_DESC translatedDesc = RBUtil::TranslateUAVDesc( inResource, uavDesc );
    _device->CreateUnorderedAccessView( resource.native, nullptr, &translatedDesc, uav->handle.cpu );

    return uav;
}

IRBTextureViewTablePtr RB_DX12::CreateTextureViewTable( uint32 resourceCount )
{
    std::shared_ptr<RBTextureViewTable_DX12> table = std::make_shared<RBTextureViewTable_DX12>();
    table->handle = _heap.AllocateDynamicConsecutive( RBResourceViewType_CBV_SRV_UAV_SHADERVISIBLE, resourceCount );
    table->resourceCount = resourceCount;

    return table;
}

IRBDescriptorTableViewPtr RB_DX12::CreateDescriptorTableView( const std::vector<const IRBResource*>& inResources,
                                                                const std::vector<RBConstantBufferViewDesc>& inCBVDescs,
                                                                const std::vector<RBShaderResourceViewDesc>& inSRVDescs,
                                                                const std::vector<RBUnorderedAccessViewDesc>& inUAVDescs )
{
    assert( inResources.size() == inCBVDescs.size() + inSRVDescs.size() + inUAVDescs.size() );

    std::vector<RBDescriptorHandle_DX12> handles;
    handles.resize( inResources.size() );

    uint32 resourceIndex = 0;
    for ( uint32 descIndex = 0; descIndex < inCBVDescs.size(); ++descIndex )
    {
        const RBResource_DX12& resource = static_cast<const RBResource_DX12&>( *inResources[ resourceIndex ] );
        const RBConstantBufferViewDesc& cbvDesc = inCBVDescs[ descIndex ];
        D3D12_CONSTANT_BUFFER_VIEW_DESC translatedDesc = RBUtil::TranslateCBVDesc( resource, cbvDesc );

        handles[ resourceIndex ] = cbvDesc.bStatic ? _heap.AllocateStatic( RBResourceViewType_CBV_SRV_UAV_SHADERVISIBLE ) : _heap.AllocateDynamic( RBResourceViewType_CBV_SRV_UAV_SHADERVISIBLE );

        _device->CreateConstantBufferView( &translatedDesc, handles[ resourceIndex ].cpu );

        resourceIndex++;
    }
    for ( uint32 descIndex = 0; descIndex < inSRVDescs.size(); ++descIndex )
    {
        const RBResource_DX12& resource = static_cast<const RBResource_DX12&>( *inResources[ resourceIndex ] );
        const RBShaderResourceViewDesc& srvDesc = inSRVDescs[ descIndex ];
        D3D12_SHADER_RESOURCE_VIEW_DESC translatedDesc = RBUtil::TranslateSRVDesc( resource, srvDesc );

        handles[ resourceIndex ] = srvDesc.bStatic ? _heap.AllocateStatic( RBResourceViewType_CBV_SRV_UAV_SHADERVISIBLE ) : _heap.AllocateDynamic( RBResourceViewType_CBV_SRV_UAV_SHADERVISIBLE );

        _device->CreateShaderResourceView( resource.native, &translatedDesc, handles[ resourceIndex ].cpu );

        resourceIndex++;
    }
    for ( uint32 descIndex = 0; descIndex < inUAVDescs.size(); ++descIndex )
    {
        const RBResource_DX12& resource = static_cast<const RBResource_DX12&>( *inResources[ resourceIndex ] );
        const RBUnorderedAccessViewDesc& uavDesc = inUAVDescs[ descIndex ];
        D3D12_UNORDERED_ACCESS_VIEW_DESC translatedDesc = RBUtil::TranslateUAVDesc( resource, inUAVDescs[ descIndex ] );

        handles[ resourceIndex ] = uavDesc.bStatic ? _heap.AllocateStatic( RBResourceViewType_CBV_SRV_UAV_SHADERVISIBLE ) : _heap.AllocateDynamic( RBResourceViewType_CBV_SRV_UAV_SHADERVISIBLE );

        _device->CreateUnorderedAccessView( resource.native, nullptr, &translatedDesc, handles[ resourceIndex ].cpu );

        resourceIndex++;
    }

    std::shared_ptr<RBDescriptorTableView_DX12> table = std::make_shared<RBDescriptorTableView_DX12>();
    table->handle = handles[ 0 ];

    //#pragma warning "[Danger] There are possibilities of sparse memory allocation!"

    return table;
}

IRBSamplerPtr RB_DX12::CreateSampler( const IRBResource& inResource, const RBSamplerDesc& samplerDesc )
{
    return nullptr;
}

IRBVertexBufferViewPtr RB_DX12::CreateVertexBufferView( const IRBResource& inResource, const uint32 size, const uint32 stride )
{
    const RBResource_DX12& resource = static_cast<const RBResource_DX12&>( inResource );

    std::shared_ptr<RBVertexBufferView_DX12> vb = std::make_shared<RBVertexBufferView_DX12>();
    vb->native.BufferLocation = resource.native->GetGPUVirtualAddress();
    vb->native.SizeInBytes = size;
    vb->native.StrideInBytes = stride;

    return vb;
}

IRBIndexBufferViewPtr RB_DX12::CreateIndexBufferView( const IRBResource& inResource, const uint32 size )
{
    const RBResource_DX12& resource = static_cast<const RBResource_DX12&>( inResource );

    std::shared_ptr<RBIndexBufferView_DX12> ib = std::make_shared<RBIndexBufferView_DX12>();
    ib->native.BufferLocation = resource.native->GetGPUVirtualAddress();
    ib->native.SizeInBytes = size;
    ib->native.Format = DXGI_FORMAT_R32_UINT;

    return ib;
}

void RB_DX12::CopyBufferRegion( const IRBResource& inCopyDst, const IRBResource& inCopySrc, const uint32 copyDstByteOffset, const uint32 copyByteSize )
{
    const RBResource_DX12& copySrc = static_cast<const RBResource_DX12&>( inCopySrc );
    const RBResource_DX12& copyDst = static_cast<const RBResource_DX12&>( inCopyDst );

    CommandQueueContext& cmdQueueCtx = _cmdQueueCtx[ D3D12_COMMAND_LIST_TYPE_DIRECT ];
    ID3D12GraphicsCommandList* cmdList = *cmdQueueCtx.iCmdList;

    TransitionResource( cmdList, copySrc.native, RBUtil::TranslateResourceState( copySrc.state ), D3D12_RESOURCE_STATE_COPY_SOURCE );;
    TransitionResource( cmdList, copyDst.native, RBUtil::TranslateResourceState( copyDst.state ), D3D12_RESOURCE_STATE_COPY_DEST );

    cmdList->CopyBufferRegion( copyDst.native, copyDstByteOffset, copySrc.native, 0, copyByteSize );

    TransitionResource( cmdList, copySrc.native, D3D12_RESOURCE_STATE_COPY_SOURCE, RBUtil::TranslateResourceState( copySrc.state ) );
    TransitionResource( cmdList, copyDst.native, D3D12_RESOURCE_STATE_COPY_DEST, RBUtil::TranslateResourceState( copyDst.state ) );

    ExecuteCommandList();
}

void RB_DX12::CopyTextureRegion( const IRBResource& inCopyDst, const IRBResource& inCopySrc, uint32 width, uint32 height )
{
    const RBResource_DX12& copySrc = static_cast<const RBResource_DX12&>( inCopySrc );
    const RBResource_DX12& copyDst = static_cast<const RBResource_DX12&>( inCopyDst );

    CommandQueueContext& cmdQueueCtx = _cmdQueueCtx[ D3D12_COMMAND_LIST_TYPE_DIRECT ];
    ID3D12GraphicsCommandList* cmdList = *cmdQueueCtx.iCmdList;

    TransitionResource( cmdList, copySrc.native, RBUtil::TranslateResourceState( copySrc.state ), D3D12_RESOURCE_STATE_COPY_SOURCE );;
    TransitionResource( cmdList, copyDst.native, RBUtil::TranslateResourceState( copyDst.state ), D3D12_RESOURCE_STATE_COPY_DEST );

    D3D12_TEXTURE_COPY_LOCATION copyDestLocation;
    copyDestLocation.pResource = copyDst.native;
    copyDestLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    copyDestLocation.SubresourceIndex = 0;

    D3D12_TEXTURE_COPY_LOCATION copySrcLocation;
    copySrcLocation.pResource = copySrc.native;
    copySrcLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    copySrcLocation.SubresourceIndex = 0;

    D3D12_BOX srcBox;
    srcBox.left = 0;
    srcBox.top = 0;
    srcBox.front = 0;
    srcBox.right = width;
    srcBox.bottom = height;
    srcBox.back = 1;

    cmdList->CopyTextureRegion( &copyDestLocation, 0, 0, 0, &copySrcLocation, &srcBox );

    TransitionResource( cmdList, copySrc.native, D3D12_RESOURCE_STATE_COPY_SOURCE, RBUtil::TranslateResourceState( copySrc.state ) );
    TransitionResource( cmdList, copyDst.native, D3D12_RESOURCE_STATE_COPY_DEST, RBUtil::TranslateResourceState( copyDst.state ) );

    ExecuteCommandList();
}

void RB_DX12::BindConstantBufferView( const IRBConstantBufferView& inCBV, uint32 index )
{
    const RBConstantBufferView_DX12& cbv = static_cast<const RBConstantBufferView_DX12&>( inCBV );

    CommandQueueContext& cmdQueueCtx = _cmdQueueCtx[ D3D12_COMMAND_LIST_TYPE_DIRECT ];
    ID3D12GraphicsCommandList* cmdList = *cmdQueueCtx.iCmdList;

    cmdList->SetGraphicsRootConstantBufferView( index, cbv.gpuAddress );
}

void RB_DX12::BindShaderResourceView( const IRBShaderResourceView& inSRV, uint32 index )
{
    const RBShaderResourceView_DX12& srv = static_cast<const RBShaderResourceView_DX12&>( inSRV );

    CommandQueueContext& cmdQueueCtx = _cmdQueueCtx[ D3D12_COMMAND_LIST_TYPE_DIRECT ];
    ID3D12GraphicsCommandList* cmdList = *cmdQueueCtx.iCmdList;

    cmdList->SetGraphicsRootShaderResourceView( index, srv.gpuAddress );
}

void RB_DX12::BindUnorderedAccessView( const IRBUnorderedAccessView& inUAV, uint32 index )
{
    const RBUnorderedAccessView_DX12& uav = static_cast<const RBUnorderedAccessView_DX12&>( inUAV );

    CommandQueueContext& cmdQueueCtx = _cmdQueueCtx[ D3D12_COMMAND_LIST_TYPE_DIRECT ];
    ID3D12GraphicsCommandList* cmdList = *cmdQueueCtx.iCmdList;

    cmdList->SetGraphicsRootUnorderedAccessView( index, uav.gpuAddress );
}

void RB_DX12::BindTextureViewTable( const IRBTextureViewTable& inTable, const uint32 index )
{
    const RBTextureViewTable_DX12& table = static_cast<const RBTextureViewTable_DX12&>( inTable );

    CommandQueueContext& cmdQueueCtx = _cmdQueueCtx[ D3D12_COMMAND_LIST_TYPE_DIRECT ];
    ID3D12GraphicsCommandList* cmdList = *cmdQueueCtx.iCmdList;

    ID3D12DescriptorHeap* textureHeap = _heap.GetHeap( RBResourceViewType_CBV_SRV_UAV_SHADERVISIBLE );
    cmdList->SetDescriptorHeaps( 1, &textureHeap );

    cmdList->SetGraphicsRootDescriptorTable( index, table.handle.gpu );
}

void RB_DX12::BindRenderTargetViews( std::span<const IRBRenderTargetView*> inRTVs, const IRBDepthStencilView* inDSV )
{
    const RBDepthStencilView_DX12* dsv = static_cast<const RBDepthStencilView_DX12*>( inDSV );

    std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> rtvHandles( inRTVs.size() );

    for ( int32 index = 0; index < inRTVs.size(); ++index )
    {
        const RBRenderTargetView_DX12* rtv = static_cast<const RBRenderTargetView_DX12*>( inRTVs[ index ] );
        rtvHandles[ index ] = rtv->handle.cpu;
    }

    CommandQueueContext& cmdQueueCtx = _cmdQueueCtx[ D3D12_COMMAND_LIST_TYPE_DIRECT ];
    ID3D12GraphicsCommandList* cmdList = *cmdQueueCtx.iCmdList;

    const D3D12_CPU_DESCRIPTOR_HANDLE* dsvHandle = dsv ? &dsv->handle.cpu : nullptr;
    cmdList->OMSetRenderTargets( rtvHandles.size(), rtvHandles.data(), false, dsvHandle );
}

//@TODO: memory may leak somewhere..
void RB_DX12::UpdateResourceData( const IRBResource& inResource, const void* data, uint32 sizeInBytes )
{
    const RBResource_DX12& resource = static_cast<const RBResource_DX12&>( inResource );

    D3D12_RESOURCE_DESC uploadBufferDesc = resource.native->GetDesc();
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

    if ( data )
    {
        CopyMemoryToBuffer( uploadBuffer, data, sizeInBytes );
    }

    CommandQueueContext& cmdQueueCtx = _cmdQueueCtx[ D3D12_COMMAND_LIST_TYPE_DIRECT ];
    ID3D12GraphicsCommandList* cmdList = *cmdQueueCtx.iCmdList;

    TransitionResource( cmdList, resource.native, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST );

    cmdList->CopyBufferRegion( resource.native, 0, uploadBuffer, 0, sizeInBytes );

    TransitionResource( cmdList, resource.native, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COMMON );

    ExecuteCommandList();

    uploadBuffer->Release();
}

void RB_DX12::UpdateTextureData( const IRBResource& inResource, const void* data, uint32 width, uint32 height )
{
    const RBResource_DX12& resource = static_cast<const RBResource_DX12&>( inResource );

    D3D12_RESOURCE_DESC resourceDesc = RBUtil::TranslateResourceDesc( RBUtil::GetVertexResourceDesc( "", width * height * sizeof( uint32 ) ) );
    D3D12_HEAP_PROPERTIES heapProp = HeapProperties( D3D12_HEAP_TYPE_UPLOAD );

    // Create upload buffer on CPU
    ID3D12Resource* uploadBuffer;
    CHECK_HRESULT( _device->CreateCommittedResource( &heapProp,
                                                     D3D12_HEAP_FLAG_NONE,
                                                     &resourceDesc,
                                                     D3D12_RESOURCE_STATE_GENERIC_READ,
                                                     nullptr,
                                                     IID_PPV_ARGS( &uploadBuffer ) ),
                   L"Failed to create upload buffer." );

    CopyMemoryToBuffer( uploadBuffer, data, width * height * sizeof( uint32 ) );

    CommandQueueContext& cmdQueueCtx = _cmdQueueCtx[ D3D12_COMMAND_LIST_TYPE_DIRECT ];
    ID3D12GraphicsCommandList* cmdList = *cmdQueueCtx.iCmdList;

    TransitionResource( cmdList, resource.native, RBUtil::TranslateResourceState( resource.state ), D3D12_RESOURCE_STATE_COPY_DEST );

    uint64 requiredSize = 0;
    D3D12_PLACED_SUBRESOURCE_FOOTPRINT layout;
    uint32 numRow;
    uint64 rowSizesInBytes;

    D3D12_RESOURCE_DESC outDesc = resource.native->GetDesc();
    _device->GetCopyableFootprints( &outDesc, 0, 1, 0, &layout, &numRow, &rowSizesInBytes, &requiredSize );

    D3D12_TEXTURE_COPY_LOCATION srcLocation{};
    srcLocation.pResource = uploadBuffer;
    srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
    srcLocation.PlacedFootprint = layout;

    D3D12_TEXTURE_COPY_LOCATION dstLocation{};
    dstLocation.pResource = resource.native;
    dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    dstLocation.SubresourceIndex = 0;

    cmdList->CopyTextureRegion( &dstLocation, 0, 0, 0, &srcLocation, nullptr );

    TransitionResource( cmdList, resource.native, D3D12_RESOURCE_STATE_COPY_DEST, RBUtil::TranslateResourceState( resource.state ) );

    ExecuteCommandList();

    uploadBuffer->Release();
}

void RB_DX12::UpdateTextureViewTable( const IRBTextureViewTable& inTable, const IRBShaderResourceView& inSRV, uint32 index )
{
    const RBTextureViewTable_DX12& table = static_cast<const RBTextureViewTable_DX12&>( inTable );
    const RBShaderResourceView_DX12& srv = static_cast<const RBShaderResourceView_DX12&>( inSRV );

    D3D12_CPU_DESCRIPTOR_HANDLE dst = { table.handle.cpu.ptr + index * _device->GetDescriptorHandleIncrementSize( D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV ) };
    D3D12_CPU_DESCRIPTOR_HANDLE src = srv.handle.cpu;
    _device->CopyDescriptorsSimple( 1, dst, src, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV );
}

ID3D12Resource* RB_DX12::CreateResource_Inner( const RBResourceDesc& desc, const void* data, uint32 sizeInBytes )
{
    D3D12_RESOURCE_DESC translatedDesc = RBUtil::TranslateResourceDesc( desc );
    D3D12_HEAP_PROPERTIES defaultHeapProp = HeapProperties( D3D12_HEAP_TYPE_DEFAULT );
    D3D12_HEAP_PROPERTIES uploadHeapProp = HeapProperties( D3D12_HEAP_TYPE_UPLOAD );

    ID3D12Resource* resource;
    CHECK_HRESULT( _device->CreateCommittedResource( &defaultHeapProp,
                                                     D3D12_HEAP_FLAG_NONE,
                                                     &translatedDesc,
                                                     D3D12_RESOURCE_STATE_COMMON,
                                                     nullptr,
                                                     IID_PPV_ARGS( &resource ) ),
                   L"Failed to create output buffer." );

    if ( !desc.name.empty() )
    {
        std::wstring name;
        name.assign( desc.name.begin(), desc.name.end() );

        resource->SetName( name.c_str() );
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

    if ( data )
    {
        CopyMemoryToBuffer( uploadResource, data, sizeInBytes );
    }

    CommandQueueContext& cmdQueueCtx = _cmdQueueCtx[ D3D12_COMMAND_LIST_TYPE_DIRECT ];
    ID3D12GraphicsCommandList* cmdList = *cmdQueueCtx.iCmdList;

    TransitionResource( cmdList, resource, RBUtil::TranslateResourceState( desc.initialState ), D3D12_RESOURCE_STATE_COPY_DEST );

    cmdList->CopyBufferRegion( resource, 0, uploadResource, 0, sizeInBytes );

    TransitionResource( cmdList, resource, D3D12_RESOURCE_STATE_COPY_DEST, RBUtil::TranslateResourceState( desc.initialState ) );

    ExecuteCommandList();

    uploadResource->Release();

    return resource;
}

void RB_DX12::TransitionResource( ID3D12GraphicsCommandList* cmdList, ID3D12Resource* resource, D3D12_RESOURCE_STATES stateBefore, D3D12_RESOURCE_STATES stateAfter )
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

void RB_DX12::TransitionResource( const IRBResource& inResource, const ERBResourceStates statesBefore, const ERBResourceStates statesAfter )
{
    const RBResource_DX12& resource = static_cast<const RBResource_DX12&>( inResource );

    CommandQueueContext& cmdQueueCtx = _cmdQueueCtx[ D3D12_COMMAND_LIST_TYPE_DIRECT ];
    ID3D12GraphicsCommandList* cmdList = *cmdQueueCtx.iCmdList;

    TransitionResource( cmdList, resource.native, RBUtil::TranslateResourceState( statesBefore ), RBUtil::TranslateResourceState( statesAfter ) );
}
