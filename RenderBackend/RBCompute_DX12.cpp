#include "RB_DX12.h"

//static ID3D12RootSignature* CreateComputeRootSignature( ID3D12Device* device, const RBPipelineStateDesc& pipelineDesc )
//{
//    ID3D12RootSignature* rootSignature;
//
//    D3D12_DESCRIPTOR_RANGE descRange{};
//    descRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
//    descRange.NumDescriptors = 1;
//    descRange.BaseShaderRegister = 0;
//
//    D3D12_ROOT_PARAMETER rootParam{};
//    rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
//    rootParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
//    rootParam.DescriptorTable.NumDescriptorRanges = 1;
//    rootParam.DescriptorTable.pDescriptorRanges = &descRange;
//
//    D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
//    rootSignatureDesc.NumParameters = 1;
//    rootSignatureDesc.pParameters = &rootParam;
//    rootSignatureDesc.NumStaticSamplers = 0;
//    rootSignatureDesc.pStaticSamplers = nullptr;
//    rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
//
//    ID3DBlob* rootBlob;
//    ID3DBlob* errorBlob;
//    CHECK_HRESULT( D3D12SerializeRootSignature( &rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &rootBlob, &errorBlob ),
//                   L"Failed to serialize root signature" );
//
//    CHECK_HRESULT( device->CreateRootSignature( 0, rootBlob->GetBufferPointer(), rootBlob->GetBufferSize(), IID_PPV_ARGS( &rootSignature ) ),
//                   L"Failed to create root signature" );
//
//    return rootSignature;
//}
//
//static ID3D12PipelineState* CreateComputePipelineState( ID3D12Device* device, ID3D12RootSignature* rootSignature, ID3DBlob* computeShader )
//{
//    ID3D12PipelineState* pso;
//
//    D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc{};
//    psoDesc.pRootSignature = rootSignature;
//    psoDesc.CS.BytecodeLength = computeShader->GetBufferSize();
//    psoDesc.CS.pShaderBytecode = computeShader->GetBufferPointer();
//
//    device->CreateComputePipelineState( &psoDesc, IID_PPV_ARGS( &pso ) );
//
//    return pso;
//}

void RB_DX12::RunCS()
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
