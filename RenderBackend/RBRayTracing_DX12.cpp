#include "RB_DX12.h"

#include <RenderBackend/RBPipeline_DX12.h>
//
//static ID3D12RootSignature* CreateRayTraceRootSignature( ID3D12Device5* device, const RBPipelineStateDesc& pipelineDesc )
//{
//    ID3D12RootSignature* rootSignature;
//
//    /*D3D12_DESCRIPTOR_RANGE descTable[ 2 ]{};
//    descTable[ 0 ].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
//    descTable[ 0 ].NumDescriptors = 1;
//    descTable[ 0 ].BaseShaderRegister = 5;
//    descTable[ 0 ].RegisterSpace = 0;
//    descTable[ 0 ].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
//    descTable[ 1 ].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
//    descTable[ 1 ].NumDescriptors = 1;
//    descTable[ 1 ].BaseShaderRegister = 0;
//    descTable[ 1 ].RegisterSpace = 0;
//    descTable[ 1 ].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
//
//    D3D12_ROOT_PARAMETER rootParams[ 7 ]{};
//    rootParams[ 0 ].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
//    rootParams[ 0 ].DescriptorTable = { 2, descTable };
//    rootParams[ 0 ].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
//    rootParams[ 1 ].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
//    rootParams[ 1 ].Descriptor.ShaderRegister = 0;
//    rootParams[ 1 ].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
//    rootParams[ 2 ].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
//    rootParams[ 2 ].Descriptor.ShaderRegister = 1;
//    rootParams[ 2 ].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
//    rootParams[ 3 ].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
//    rootParams[ 3 ].Descriptor.ShaderRegister = 2;
//    rootParams[ 3 ].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
//    rootParams[ 4 ].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
//    rootParams[ 4 ].Descriptor.ShaderRegister = 3;
//    rootParams[ 4 ].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
//    rootParams[ 5 ].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
//    rootParams[ 5 ].Descriptor.ShaderRegister = 4;
//    rootParams[ 5 ].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
//    rootParams[ 6 ].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
//    rootParams[ 6 ].Descriptor.ShaderRegister = 0;
//    rootParams[ 6 ].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
//
//    D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
//    rootSignatureDesc.NumParameters = 7;
//    rootSignatureDesc.pParameters = rootParams;
//    rootSignatureDesc.NumStaticSamplers = 0;
//    rootSignatureDesc.pStaticSamplers = nullptr;
//    rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;
//
//    ID3DBlob* rootBlob;
//    ID3DBlob* errorBlob;
//    CHECK_HRESULT( D3D12SerializeRootSignature( &rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &rootBlob, &errorBlob ),
//                   L"Failed to serialize root signature" );
//    if ( errorBlob )
//    {
//        std::string errorMsg = (char*)errorBlob->GetBufferPointer();
//        std::wstring wErrorMsg;
//        wErrorMsg.assign( errorMsg.begin(), errorMsg.end() );
//        AEMessageBox( wErrorMsg );
//    }
//
//    CHECK_HRESULT( device->CreateRootSignature( 0, rootBlob->GetBufferPointer(), rootBlob->GetBufferSize(), IID_PPV_ARGS( &rootSignature ) ),
//                   L"Failed to create root signature" );*/
//
//    return rootSignature;
//}

void RB_DX12::SetRayTracePipelineState( const RBPipeline_DX12& pipeline )
{
    /*CommandQueueContext& cmdQueueCtx = _cmdQueueCtx[ D3D12_COMMAND_LIST_TYPE_COMPUTE ];
    ID3D12CommandAllocator* cmdAllocator = cmdQueueCtx.allocator;
    ID3D12GraphicsCommandList4* cmdList = ( ID3D12GraphicsCommandList4* )*cmdQueueCtx.iCmdList;

    CHECK_HRESULT( cmdAllocator->Reset(), L"Failed to reset allocator." );
    CHECK_HRESULT( cmdList->Reset( cmdAllocator, nullptr ), L"Failed to reset command list." );

    cmdList->SetComputeRootSignature( pipeline.rootSignature );
    cmdList->SetPipelineState1( pipeline.raytrace.pipelineState );

    ID3D12DescriptorHeap* rtvHeap = _heap.GetHeap( RBResourceViewType_RTV );
    cmdList->SetDescriptorHeaps( 1, &rtvHeap );

    uint32 rootParamIndex = 0;
    for( uint32 index = 0; index < pipeline.rtvGPU.size(); ++index )
    {
        cmdList->SetComputeRootDescriptorTable( rootParamIndex++, pipeline.rtvGPU[ index ] );
    }

    D3D12_DISPATCH_RAYS_DESC dispatchRays{};
    dispatchRays.RayGenerationShaderRecord.StartAddress = pipeline.raytrace.resource->GetGPUVirtualAddress();
    dispatchRays.RayGenerationShaderRecord.SizeInBytes = pipeline.raytrace.resourceByteSize;
    dispatchRays.Width = _windowSize.x;
    dispatchRays.Height = _windowSize.y;
    dispatchRays.Depth = 1;
    cmdList->DispatchRays( &dispatchRays );*/
}

//IRBPipelineRef RB_DX12::CreateRaytracingPipelineState( const RBPipelineStateDesc& pipelineDesc )
//{
//    assert( pipelineDesc.pipelineType == PipelineType_RayTrace );
//
//    if ( _pipelineCache.contains( pipelineDesc.id ) )
//    {
//        return nullptr;
//    }
//
//    std::shared_ptr< RBPipeline_DX12 > pipelineState = std::make_shared<RBPipeline_DX12>();
//    //_pipelineCache.emplace( pipelineDesc.id, pipelineState );
//
//    //IDxcBlob* shaderBlob = CreateShader2( pipelineDesc.raytraceShader );
//
//    //pipelineState->rootSignature = CreateRayTraceRootSignature( (ID3D12Device5*)_device, pipelineDesc );
//
//    //D3D12_EXPORT_DESC shaderExport[ 4 ]
//    //{
//    //    { L"RayGeneration", nullptr, D3D12_EXPORT_FLAG_NONE },
//    //    { L"Hit", nullptr, D3D12_EXPORT_FLAG_NONE },
//    //    { L"Miss", nullptr, D3D12_EXPORT_FLAG_NONE },
//    //    { L"ShadowMiss", nullptr, D3D12_EXPORT_FLAG_NONE }
//    //};
//    //D3D12_DXIL_LIBRARY_DESC shaderLibrary{};
//    //shaderLibrary.DXILLibrary.pShaderBytecode = shaderBlob->GetBufferPointer();
//    //shaderLibrary.DXILLibrary.BytecodeLength = shaderBlob->GetBufferSize();
//    //shaderLibrary.NumExports = 4;
//    //shaderLibrary.pExports = shaderExport;
//    //D3D12_STATE_SUBOBJECT shader
//    //{
//    //    D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY,
//    //    &shaderLibrary
//    //};
//    //D3D12_HIT_GROUP_DESC hitGroup
//    //{
//    //    L"HitGroup0",
//    //    D3D12_HIT_GROUP_TYPE_TRIANGLES,
//    //    nullptr,
//    //    L"Hit",
//    //    nullptr
//    //};
//    //D3D12_STATE_SUBOBJECT hitGroupObject
//    //{
//    //    D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP,
//    //    &hitGroup
//    //};
//    //D3D12_RAYTRACING_SHADER_CONFIG shaderConfig
//    //{
//    //    sizeof( float[ 5 ] ),
//    //    sizeof( float[ 2 ] )
//    //};
//    //D3D12_STATE_SUBOBJECT config
//    //{
//    //    D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG,
//    //    &shaderConfig
//    //};
//    //D3D12_STATE_SUBOBJECT rootSignature
//    //{
//    //    D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE,
//    //    &pipelineState->rootSignature
//    //};
//    //D3D12_RAYTRACING_PIPELINE_CONFIG pipelineConfig
//    //{
//    //    10
//    //};
//    //D3D12_STATE_SUBOBJECT pipeline
//    //{
//    //    D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG,
//    //    &pipelineConfig
//    //};
//    //D3D12_STATE_SUBOBJECT subObjects[]
//    //{
//    //    shader,
//    //    hitGroupObject,
//    //    config,
//    //    rootSignature,
//    //    pipeline
//    //};
//    //D3D12_STATE_OBJECT_DESC raytracingStateObject{
//    //    D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE,
//    //    5,
//    //    subObjects
//    //};
//
//    //CHECK_HRESULT( ( (ID3D12Device5*)_device )->CreateStateObject( &raytracingStateObject, IID_PPV_ARGS( &pipelineState->raytrace.pipelineState ) ),
//    //               L"Failed to create raytrace PSO." );
//
//    //pipelineState->raytrace.pipelineState->QueryInterface( IID_PPV_ARGS( &pipelineState->raytrace.stateProperties ) );
//
//    //__declspec( align( 256 ) )
//    //    struct ShaderParam1
//    //{
//    //    uint8 shaderIdentifier[ D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES ];
//    //    uint64 rootParams[ 1 ];
//    //} shaderParam;
//    //ShaderParam1 shaderParam1[ 2 ];
//
//    //// ray generation
//    //memcpy( shaderParam.shaderIdentifier, pipelineState->raytrace.stateProperties->GetShaderIdentifier( L"RayGeneration" ), D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES );
//
//    //RBResourceDesc desc{};
//    //desc.name = L"RayTraceTest_RayGeneration";
//    //desc.dimension = ERBResourceDimension::Buffer;
//    //desc.format = ERBResourceFormat::Unknown;
//    //desc.width = sizeof( shaderParam );
//    //desc.height = 1;
//    //desc.depth = 1;
//    //desc.numMips = 1;
//    //desc.flags = RBResourceFlag_None;
//
//    //pipelineState->raytrace.resource = CreateResource_Inner( desc, &shaderParam, sizeof( shaderParam ) );
//    //pipelineState->raytrace.resourceByteSize = sizeof( shaderParam );
//
//    //// 
//    //memcpy( shaderParam.shaderIdentifier, pipelineState->raytrace.stateProperties->GetShaderIdentifier( L"HitGroup0" ), D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES );
//    //desc.name = L"RayTraceTest_HitGroup";
//    //pipelineState->raytrace.resource1 = CreateResource_Inner( desc, &shaderParam, sizeof( shaderParam ) );
//
//    //// 
//    //memcpy( shaderParam1[ 0 ].shaderIdentifier, pipelineState->raytrace.stateProperties->GetShaderIdentifier( L"Miss" ), D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES );
//    //memcpy( shaderParam1[ 1 ].shaderIdentifier, pipelineState->raytrace.stateProperties->GetShaderIdentifier( L"ShadowMiss" ), D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES );
//    //desc.name = L"RayTraceTest_Miss";
//    //desc.width = sizeof( shaderParam1 );
//    //pipelineState->raytrace.resource2 = CreateResource_Inner( desc, &shaderParam1, sizeof( shaderParam1 ) );
//
//    /////////////////////////////////////////////////////////////
//    //pipelineState->uav.resize( pipelineDesc.numUAVs );
//    //pipelineState->uavHandle.resize( pipelineDesc.numUAVs );
//    //pipelineState->cbv.resize( pipelineDesc.numCBVs );
//
//    return pipelineState;
//}

IRBRayTraceBottomLevelASRef RB_DX12::CreateRayTraceBottomLevelAS( const RBRayTraceBottomLevelASDesc& asDesc, const IRBVertexBufferView& inVBV, const IRBIndexBufferView& inIBV )
{
    std::shared_ptr<RBRayTraceBottomLevelAS_DX12> as = std::make_shared<RBRayTraceBottomLevelAS_DX12>();

    //CommandQueueContext& cmdQueueCtx = _cmdQueueCtx[ D3D12_COMMAND_LIST_TYPE_COMPUTE ];
    //ID3D12CommandAllocator* cmdAllocator = cmdQueueCtx.allocator;
    //ID3D12GraphicsCommandList4* cmdList = (ID3D12GraphicsCommandList4*)*cmdQueueCtx.iCmdList;

    //CHECK_HRESULT( cmdAllocator->Reset(), L"Failed to reset allocator." );
    //CHECK_HRESULT( cmdList->Reset( cmdAllocator, nullptr ), L"Failed to reset command list." );

    //const RBVertexBufferView_DX12& vbv = static_cast<const RBVertexBufferView_DX12&>( inVBV );
    //const RBIndexBufferView_DX12& ibv = static_cast<const RBIndexBufferView_DX12&>( inIBV );

    //// create bottom-level acceleration structure
    //D3D12_RAYTRACING_GEOMETRY_TRIANGLES_DESC triangles{};
    //triangles.VertexBuffer.StartAddress = vbv.gpuAddress;
    //triangles.VertexBuffer.StrideInBytes = vbv.stride;
    //triangles.VertexCount = vbv.size / vbv.stride;
    //triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;

    //triangles.IndexBuffer = ibv.gpuAddress;
    //triangles.IndexCount = ibv.size / sizeof( uint32 );
    //triangles.IndexFormat = DXGI_FORMAT_R32_UINT;

    //triangles.Transform3x4 = 0;

    //D3D12_RAYTRACING_GEOMETRY_DESC rtrDesc{};
    //rtrDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
    //rtrDesc.Triangles = triangles;
    //rtrDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_NONE;

    //D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS asInputs{};
    //asInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
    //asInputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE;
    //asInputs.NumDescs = 1;
    //asInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
    //asInputs.pGeometryDescs = &rtrDesc;

    //D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO prebuildInfo{};
    //( (ID3D12Device5*)_device )->GetRaytracingAccelerationStructurePrebuildInfo( &asInputs, &prebuildInfo );

    //ID3D12Resource* sbResource;
    //D3D12_RESOURCE_DESC sbResourceDesc{};
    //sbResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    //sbResourceDesc.Alignment = 0;
    //sbResourceDesc.Width = prebuildInfo.ScratchDataSizeInBytes;
    //sbResourceDesc.Height = 1;
    //sbResourceDesc.DepthOrArraySize = 1;
    //sbResourceDesc.MipLevels = 1;
    //sbResourceDesc.Format = DXGI_FORMAT_UNKNOWN;
    //sbResourceDesc.SampleDesc.Count = 1;
    //sbResourceDesc.SampleDesc.Quality = 0;
    //sbResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    //sbResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

    //D3D12_HEAP_PROPERTIES defaultHeapProp = HeapProperties( D3D12_HEAP_TYPE_DEFAULT );

    //CHECK_HRESULT( _device->CreateCommittedResource( &defaultHeapProp,
    //                                                 D3D12_HEAP_FLAG_NONE,
    //                                                 &sbResourceDesc,
    //                                                 D3D12_RESOURCE_STATE_COMMON,
    //                                                 nullptr,
    //                                                 IID_PPV_ARGS( &sbResource ) ),
    //               L"Failed to create output buffer." );
    //sbResource->SetName( L"BLAS_SB" );

    //D3D12_RESOURCE_DESC asResourceDesc{};
    //asResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    //asResourceDesc.Alignment = 0;
    //asResourceDesc.Width = prebuildInfo.ResultDataMaxSizeInBytes;
    //asResourceDesc.Height = 1;
    //asResourceDesc.DepthOrArraySize = 1;
    //asResourceDesc.MipLevels = 1;
    //asResourceDesc.Format = DXGI_FORMAT_UNKNOWN;
    //asResourceDesc.SampleDesc.Count = 1;
    //asResourceDesc.SampleDesc.Quality = 0;
    //asResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    //asResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

    //CHECK_HRESULT( _device->CreateCommittedResource( &defaultHeapProp,
    //                                                 D3D12_HEAP_FLAG_NONE,
    //                                                 &asResourceDesc,
    //                                                 D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
    //                                                 nullptr,
    //                                                 IID_PPV_ARGS( &as->asResource ) ),
    //               L"Failed to create output buffer." );
    //as->asResource->SetName( L"BLAS" );

    //const RBResourceDesc cbDesc = RBUtil::GetConstantBufferResourceDesc( L"BLASC", 512 );
    //CreateResource( cbDesc );

    //D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC buildDesc{};
    //buildDesc.DestAccelerationStructureData = as->asResource->GetGPUVirtualAddress();
    //buildDesc.Inputs = asInputs;
    //buildDesc.ScratchAccelerationStructureData = sbResource->GetGPUVirtualAddress();

    //cmdList->BuildRaytracingAccelerationStructure( &buildDesc, 0, nullptr );

    //CHECK_HRESULT( cmdList->Close(), L"Failed to close command list." );

    //ID3D12CommandList* cmdListInterface = cmdList;
    //cmdQueueCtx.cmdQueue->ExecuteCommandLists( 1, &cmdListInterface );

    //WaitForFence( cmdQueueCtx );

    //as->transform = asDesc.transform;
    //as->gpuAddress = as->asResource->GetGPUVirtualAddress();

    return as;
}

IRBRayTraceTopLevelASRef RB_DX12::CreateRayTraceTopLevelAS( const std::vector<IRBRayTraceBottomLevelASRef>& inBottomLevelAS, const RBRayTraceTopLevelASDesc& asDesc )
{
    std::shared_ptr<RBRayTraceTopLevelAS_DX12> as = std::make_shared<RBRayTraceTopLevelAS_DX12>();

//    as->bottomLevel.resize( inBottomLevelAS.size() );
//    for ( uint32 index = 0; index < inBottomLevelAS.size(); ++index )
//    {
//        as->bottomLevel[ index ] = std::static_pointer_cast<RBRayTraceBottomLevelAS_DX12>( inBottomLevelAS[ index ] );
//    }
//
//    CommandQueueContext& cmdQueueCtx = _cmdQueueCtx[ D3D12_COMMAND_LIST_TYPE_COMPUTE ];
//    ID3D12CommandAllocator* cmdAllocator = cmdQueueCtx.allocator;
//    ID3D12GraphicsCommandList4* cmdList = (ID3D12GraphicsCommandList4*)*cmdQueueCtx.iCmdList;
//
//    CHECK_HRESULT( cmdAllocator->Reset(), L"Failed to reset allocator." );
//    CHECK_HRESULT( cmdList->Reset( cmdAllocator, nullptr ), L"Failed to reset command list." );
//
//    std::vector<D3D12_RAYTRACING_INSTANCE_DESC> instanceDescs( inBottomLevelAS.size() );
//    for ( uint32 index = 0; index < inBottomLevelAS.size(); ++index )
//    {
//        std::shared_ptr<RBRayTraceBottomLevelAS_DX12> bottomLevelAS = std::static_pointer_cast<RBRayTraceBottomLevelAS_DX12>( inBottomLevelAS[ index ] );
//
//        D3D12_RAYTRACING_INSTANCE_DESC& desc = instanceDescs[ index ];
//
//#define FILL_TRANSFORM(row, col) desc.Transform[ row ][ col ] = bottomLevelAS->transform.mat.r[ row ].m128_f32[ col ];
//        FILL_TRANSFORM( 0, 0 ); FILL_TRANSFORM( 0, 1 ); FILL_TRANSFORM( 0, 2 ); FILL_TRANSFORM( 0, 3 );
//        FILL_TRANSFORM( 1, 0 ); FILL_TRANSFORM( 1, 1 ); FILL_TRANSFORM( 1, 2 ); FILL_TRANSFORM( 1, 3 );
//        FILL_TRANSFORM( 2, 0 ); FILL_TRANSFORM( 2, 1 ); FILL_TRANSFORM( 2, 2 ); FILL_TRANSFORM( 2, 3 );
//#undef FILL_TRANSFORM
//
//        desc.InstanceID = index;
//        desc.InstanceMask = 0xFF;
//        desc.InstanceContributionToHitGroupIndex = 0;
//        desc.Flags = D3D12_RAYTRACING_INSTANCE_FLAG_NONE;
//        desc.AccelerationStructure = bottomLevelAS->gpuAddress;
//    }
//
//    ID3D12Resource* instanceDescResource;
//    D3D12_RESOURCE_DESC instanceDescResourceDesc{};
//    instanceDescResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
//    instanceDescResourceDesc.Alignment = 0;
//    instanceDescResourceDesc.Width = sizeof( D3D12_RAYTRACING_INSTANCE_DESC ) * inBottomLevelAS.size();
//    instanceDescResourceDesc.Height = 1;
//    instanceDescResourceDesc.DepthOrArraySize = 1;
//    instanceDescResourceDesc.MipLevels = 1;
//    instanceDescResourceDesc.Format = DXGI_FORMAT_UNKNOWN;
//    instanceDescResourceDesc.SampleDesc.Count = 1;
//    instanceDescResourceDesc.SampleDesc.Quality = 0;
//    instanceDescResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
//    instanceDescResourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
//
//    D3D12_HEAP_PROPERTIES defaultHeapProp = HeapProperties( D3D12_HEAP_TYPE_DEFAULT );
//
//    CHECK_HRESULT( _device->CreateCommittedResource( &defaultHeapProp,
//                                                     D3D12_HEAP_FLAG_NONE,
//                                                     &instanceDescResourceDesc,
//                                                     D3D12_RESOURCE_STATE_COMMON,
//                                                     nullptr,
//                                                     IID_PPV_ARGS( &instanceDescResource ) ),
//                   L"Failed to create output buffer." );
//    instanceDescResource->SetName( L"TLAS_InstanceDesc" );
//
//    {
//        D3D12_RESOURCE_DESC uploadBufferDesc = instanceDescResource->GetDesc();
//        D3D12_HEAP_PROPERTIES uploadHeapProp = HeapProperties( D3D12_HEAP_TYPE_UPLOAD );
//
//        // Create upload buffer on CPU
//        ID3D12Resource* uploadBuffer;
//        CHECK_HRESULT( _device->CreateCommittedResource( &uploadHeapProp,
//                                                         D3D12_HEAP_FLAG_NONE,
//                                                         &uploadBufferDesc,
//                                                         D3D12_RESOURCE_STATE_GENERIC_READ,
//                                                         nullptr,
//                                                         IID_PPV_ARGS( &uploadBuffer ) ),
//                       L"Failed to create upload buffer." );
//
//        CopyMemoryToBuffer( uploadBuffer, instanceDescs.data(), sizeof( D3D12_RAYTRACING_INSTANCE_DESC ) * inBottomLevelAS.size() );
//
//        TransitionResource( cmdList, instanceDescResource, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST );
//
//        cmdList->CopyBufferRegion( instanceDescResource, 0, uploadBuffer, 0, sizeof( D3D12_RAYTRACING_INSTANCE_DESC ) * inBottomLevelAS.size() );
//
//        TransitionResource( cmdList, instanceDescResource, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COMMON );
//    }
//
//    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS asInputs{};
//    asInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
//    asInputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE;
//    asInputs.NumDescs = inBottomLevelAS.size();
//    asInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
//    asInputs.InstanceDescs = instanceDescResource->GetGPUVirtualAddress();
//
//    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO prebuildInfo{};
//    ( (ID3D12Device5*)_device )->GetRaytracingAccelerationStructurePrebuildInfo( &asInputs, &prebuildInfo );
//
//    ID3D12Resource* sbResource;
//    D3D12_RESOURCE_DESC sbResourceDesc{};
//    sbResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
//    sbResourceDesc.Alignment = 0;
//    sbResourceDesc.Width = prebuildInfo.ScratchDataSizeInBytes;
//    sbResourceDesc.Height = 1;
//    sbResourceDesc.DepthOrArraySize = 1;
//    sbResourceDesc.MipLevels = 1;
//    sbResourceDesc.Format = DXGI_FORMAT_UNKNOWN;
//    sbResourceDesc.SampleDesc.Count = 1;
//    sbResourceDesc.SampleDesc.Quality = 0;
//    sbResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
//    sbResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
//
//    CHECK_HRESULT( _device->CreateCommittedResource( &defaultHeapProp,
//                                                     D3D12_HEAP_FLAG_NONE,
//                                                     &sbResourceDesc,
//                                                     D3D12_RESOURCE_STATE_COMMON,
//                                                     nullptr,
//                                                     IID_PPV_ARGS( &sbResource ) ),
//                   L"Failed to create output buffer." );
//    sbResource->SetName( L"TLAS_SB" );
//
//    D3D12_RESOURCE_DESC asResourceDesc{};
//    asResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
//    asResourceDesc.Alignment = 0;
//    asResourceDesc.Width = prebuildInfo.ResultDataMaxSizeInBytes;
//    asResourceDesc.Height = 1;
//    asResourceDesc.DepthOrArraySize = 1;
//    asResourceDesc.MipLevels = 1;
//    asResourceDesc.Format = DXGI_FORMAT_UNKNOWN;
//    asResourceDesc.SampleDesc.Count = 1;
//    asResourceDesc.SampleDesc.Quality = 0;
//    asResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
//    asResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
//
//    CHECK_HRESULT( _device->CreateCommittedResource( &defaultHeapProp,
//                                                     D3D12_HEAP_FLAG_NONE,
//                                                     &asResourceDesc,
//                                                     D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
//                                                     nullptr,
//                                                     IID_PPV_ARGS( &as->asResource ) ),
//                   L"Failed to create output buffer." );
//    as->asResource->SetName( L"TLAS" );
//
//    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC buildDesc{};
//    buildDesc.DestAccelerationStructureData = as->asResource->GetGPUVirtualAddress();
//    buildDesc.Inputs = asInputs;
//    buildDesc.ScratchAccelerationStructureData = sbResource->GetGPUVirtualAddress();
//
//    cmdList->BuildRaytracingAccelerationStructure( &buildDesc, 0, nullptr );
//
//    CHECK_HRESULT( cmdList->Close(), L"Failed to close command list." );
//
//    ID3D12CommandList* cmdListInterface = cmdList;
//    cmdQueueCtx.cmdQueue->ExecuteCommandLists( 1, &cmdListInterface );
//
//    WaitForFence( cmdQueueCtx );
//
//    as->gpuAddress = as->asResource->GetGPUVirtualAddress();

    return as;
}

//void RB_DX12::RayTrace( const IRBRayTraceTopLevelASRef& inRTRAS, IRBDescriptorTableViewRef descTableView, IRBShaderResourceViewPtr testNormalSRV, IRBShaderResourceViewPtr testIndexSRV, IRBShaderResourceViewPtr testIndexOffsetSRV, IRBShaderResourceViewPtr testMaterialSRV )
//{
    /*assert( _pipelineCache.contains( desc.id ) );

    std::shared_ptr<RBDescriptorTableView_DX12> descTableView1 = std::static_pointer_cast<RBDescriptorTableView_DX12>( descTableView );
    std::shared_ptr<RBShaderResourceView_DX12> testNormalSRV1 = std::static_pointer_cast<RBShaderResourceView_DX12>( testNormalSRV );
    std::shared_ptr<RBShaderResourceView_DX12> testIndexSRV1 = std::static_pointer_cast<RBShaderResourceView_DX12>( testIndexSRV );
    std::shared_ptr<RBShaderResourceView_DX12> testIndexOffsetSRV1 = std::static_pointer_cast<RBShaderResourceView_DX12>( testIndexOffsetSRV );
    std::shared_ptr<RBShaderResourceView_DX12> testMaterialSRV1 = std::static_pointer_cast<RBShaderResourceView_DX12>( testMaterialSRV );

    std::shared_ptr<RBPipeline_DX12>& pipeline = _pipelineCache[ desc.id ];

    CommandQueueContext& cmdQueueCtx = _cmdQueueCtx[ D3D12_COMMAND_LIST_TYPE_COMPUTE ];
    ID3D12CommandAllocator* cmdAllocator = cmdQueueCtx.allocator;
    ID3D12GraphicsCommandList4* cmdList = (ID3D12GraphicsCommandList4*)*cmdQueueCtx.iCmdList;

    CHECK_HRESULT( cmdAllocator->Reset(), L"Failed to reset allocator." );
    CHECK_HRESULT( cmdList->Reset( cmdAllocator, nullptr ), L"Failed to reset command list." );

    cmdList->SetComputeRootSignature( pipeline->rootSignature );
    cmdList->SetPipelineState1( pipeline->raytrace.pipelineState );

    ID3D12DescriptorHeap* uavHeap = _heap.GetHeap( RBResourceViewType_CBV_SRV_UAV_SHADERVISIBLE );
    cmdList->SetDescriptorHeaps( 1, &uavHeap );

    uint32 rootParamIndex = 0;
    cmdList->SetComputeRootDescriptorTable( rootParamIndex++, descTableView1->handle.gpu );

    std::shared_ptr<RBRayTraceTopLevelAS_DX12> rtrAS = std::static_pointer_cast<RBRayTraceTopLevelAS_DX12>( inRTRAS );
    cmdList->SetComputeRootShaderResourceView( rootParamIndex++, rtrAS->gpuAddress );
    cmdList->SetComputeRootShaderResourceView( rootParamIndex++, testNormalSRV1->gpuAddress );
    cmdList->SetComputeRootShaderResourceView( rootParamIndex++, testIndexSRV1->gpuAddress );
    cmdList->SetComputeRootShaderResourceView( rootParamIndex++, testIndexOffsetSRV1->gpuAddress );
    cmdList->SetComputeRootShaderResourceView( rootParamIndex++, testMaterialSRV1->gpuAddress );
    cmdList->SetComputeRootConstantBufferView( rootParamIndex++, pipeline->cbv[ 0 ] );

    D3D12_DISPATCH_RAYS_DESC dispatchRays{};
    dispatchRays.RayGenerationShaderRecord.StartAddress = pipeline->raytrace.resource->GetGPUVirtualAddress();
    dispatchRays.RayGenerationShaderRecord.SizeInBytes = pipeline->raytrace.resourceByteSize;
    dispatchRays.HitGroupTable.StartAddress = pipeline->raytrace.resource1->GetGPUVirtualAddress();
    dispatchRays.HitGroupTable.SizeInBytes = pipeline->raytrace.resourceByteSize;
    dispatchRays.HitGroupTable.StrideInBytes = pipeline->raytrace.resourceByteSize;
    dispatchRays.MissShaderTable.StartAddress = pipeline->raytrace.resource2->GetGPUVirtualAddress();
    dispatchRays.MissShaderTable.SizeInBytes = pipeline->raytrace.resourceByteSize;
    dispatchRays.MissShaderTable.StrideInBytes = pipeline->raytrace.resourceByteSize;
    dispatchRays.Width = GetWindowSize().x;
    dispatchRays.Height = GetWindowSize().y;
    dispatchRays.Depth = 1;

    cmdList->DispatchRays( &dispatchRays );

    CHECK_HRESULT( cmdList->Close(), L"Failed to close command list." );

    ID3D12CommandList* cmdListInterface = cmdList;
    cmdQueueCtx.cmdQueue->ExecuteCommandLists( 1, &cmdListInterface );

    WaitForFence( cmdQueueCtx );*/
//}
