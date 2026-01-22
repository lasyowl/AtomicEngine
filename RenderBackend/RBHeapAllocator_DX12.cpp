#include <RenderBackend/RB_DX12.h>
#include <Core/DebugUtil.h>

// todo : remove duplicated macros among different files
#define CHECK_HRESULT( hr, msg ) \
	if(FAILED (hr)) {\
		AEMessageBox( msg );\
	}

//////////////////////////////
// Misc Functions
//////////////////////////////
void CreateDescriptorHeap( ID3D12Device* device, RBDescriptorHeap_DX12& heapContext, const uint32 numDesc, const D3D12_DESCRIPTOR_HEAP_TYPE descType, const D3D12_DESCRIPTOR_HEAP_FLAGS descFlag )
{
    const bool isShaderVisible = descFlag & D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

    D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc{};
    descHeapDesc.NumDescriptors = numDesc;
    descHeapDesc.Type = descType;
    descHeapDesc.Flags = descFlag;

    CHECK_HRESULT( device->CreateDescriptorHeap( &descHeapDesc, IID_PPV_ARGS( &heapContext.heap ) ),
                   L"Failed to create descriptor heap." );

    heapContext.descSize = device->GetDescriptorHandleIncrementSize( descType );

    heapContext.hCPUStaticCurrent = heapContext.heap->GetCPUDescriptorHandleForHeapStart();
    heapContext.hGPUStaticCurrent = isShaderVisible ? heapContext.heap->GetGPUDescriptorHandleForHeapStart() : D3D12_GPU_DESCRIPTOR_HANDLE{};

    heapContext.hCPUDynamicBegin = heapContext.heap->GetCPUDescriptorHandleForHeapStart();
    heapContext.hCPUDynamicCurrent = heapContext.heap->GetCPUDescriptorHandleForHeapStart();
    heapContext.hGPUDynamicBegin = isShaderVisible ? heapContext.heap->GetGPUDescriptorHandleForHeapStart() : D3D12_GPU_DESCRIPTOR_HANDLE{};
    heapContext.hGPUDynamicCurrent = isShaderVisible ? heapContext.heap->GetGPUDescriptorHandleForHeapStart() : D3D12_GPU_DESCRIPTOR_HANDLE{};

    uint64 dynamicHandleOffset = ( numDesc / 2 ) * heapContext.descSize;
    heapContext.hCPUDynamicBegin.ptr += dynamicHandleOffset;
    heapContext.hCPUDynamicCurrent.ptr += dynamicHandleOffset;
    heapContext.hGPUDynamicBegin.ptr += dynamicHandleOffset;
    heapContext.hGPUDynamicCurrent.ptr += dynamicHandleOffset;
}

////////////////////////////////
// RBDescriptorHeapAllocator_DX12
////////////////////////////////
void RBDescriptorHeapAllocator_DX12::Initialize( ID3D12Device* device )
{
    CreateDescriptorHeap( device, _heap[ RBResourceViewType_RTV ], 256, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE );
    CreateDescriptorHeap( device, _heap[ RBResourceViewType_DSV ], 128, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE );
    CreateDescriptorHeap( device, _heap[ RBResourceViewType_CBV_SRV_UAV_SHADERHIDDEN ], 2048, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE );
    CreateDescriptorHeap( device, _heap[ RBResourceViewType_CBV_SRV_UAV_SHADERVISIBLE ], 2048, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE );
    CreateDescriptorHeap( device, _heap[ RBResourceViewType_SAMPLER ], 256, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE );
}

void RBDescriptorHeapAllocator_DX12::Release()
{
    for ( auto& heap : _heap )
    {
        heap.heap->Release();
    }
}

RBDescriptorHandle_DX12 RBDescriptorHeapAllocator_DX12::AllocateStatic( const ERBResourceViewType type )
{
    RBDescriptorHeap_DX12& heapContext = _heap[ type ];
    const RBDescriptorHandle_DX12 handle( heapContext.hCPUStaticCurrent, heapContext.hGPUStaticCurrent );
    heapContext.hCPUStaticCurrent.ptr += heapContext.descSize;
    heapContext.hGPUStaticCurrent.ptr += heapContext.descSize;

    return handle;
}

RBDescriptorHandle_DX12 RBDescriptorHeapAllocator_DX12::AllocateStaticConsecutive( const ERBResourceViewType type, const uint32 count )
{
    RBDescriptorHeap_DX12& heapContext = _heap[ type ];
    const RBDescriptorHandle_DX12 handle( heapContext.hCPUStaticCurrent, heapContext.hGPUStaticCurrent );
    heapContext.hCPUStaticCurrent.ptr += heapContext.descSize * count;
    heapContext.hGPUStaticCurrent.ptr += heapContext.descSize * count;

    return handle;
}

RBDescriptorHandle_DX12 RBDescriptorHeapAllocator_DX12::AllocateDynamic( const ERBResourceViewType type )
{
    RBDescriptorHeap_DX12& heapContext = _heap[ type ];
    const RBDescriptorHandle_DX12 handle( heapContext.hCPUDynamicCurrent, heapContext.hGPUDynamicCurrent );
    heapContext.hCPUDynamicCurrent.ptr += heapContext.descSize;
    heapContext.hGPUDynamicCurrent.ptr += heapContext.descSize;

    return handle;
}

RBDescriptorHandle_DX12 RBDescriptorHeapAllocator_DX12::AllocateDynamicConsecutive( const ERBResourceViewType type, const uint32 count )
{
    RBDescriptorHeap_DX12& heapContext = _heap[ type ];
    const RBDescriptorHandle_DX12 handle( heapContext.hCPUDynamicCurrent, heapContext.hGPUDynamicCurrent );
    heapContext.hCPUDynamicCurrent.ptr += heapContext.descSize * count;
    heapContext.hGPUDynamicCurrent.ptr += heapContext.descSize * count;

    return handle;
}

void RBDescriptorHeapAllocator_DX12::ClearDynamic()
{
    for ( RBDescriptorHeap_DX12& heapContext : _heap )
    {
        heapContext.hCPUDynamicCurrent = heapContext.hCPUDynamicBegin;
        heapContext.hGPUDynamicCurrent = heapContext.hGPUDynamicBegin;
    }
}

ID3D12DescriptorHeap* RBDescriptorHeapAllocator_DX12::GetHeap( const ERBResourceViewType type )
{
    return _heap[ type ].heap;
}
