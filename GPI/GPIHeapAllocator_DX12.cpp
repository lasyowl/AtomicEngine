#include "GPI_DX12.h"
#include <Core/DebugUtil.h>

// todo : remove duplicated macros among different files
#define CHECK_HRESULT( hr, msg ) \
	if(FAILED (hr)) {\
		AEMessageBox( msg );\
	}

//////////////////////////////
// Misc Functions
//////////////////////////////
void CreateDescriptorHeap( ID3D12Device* device, GPIDescriptorHeap_DX12& heapContext, const uint32 numDesc, const D3D12_DESCRIPTOR_HEAP_TYPE descType, const D3D12_DESCRIPTOR_HEAP_FLAGS descFlag )
{
	D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc{};
	descHeapDesc.NumDescriptors = numDesc;
	descHeapDesc.Type = descType;
	descHeapDesc.Flags = descFlag;

	CHECK_HRESULT( device->CreateDescriptorHeap( &descHeapDesc, IID_PPV_ARGS( &heapContext.heap ) ),
				   L"Failed to create descriptor heap." );

	heapContext.descSize = device->GetDescriptorHandleIncrementSize( descType );

	GPIDescriptorHeapHandle_DX12 handle;
	handle.cpu = heapContext.heap->GetCPUDescriptorHandleForHeapStart();
	handle.gpu = heapContext.heap->GetGPUDescriptorHandleForHeapStart();

	for( uint32 iter = 0; iter < numDesc; ++iter )
	{
		heapContext.free.push( handle );

		handle.cpu.ptr += heapContext.descSize;
		handle.gpu.ptr += heapContext.descSize;
	}
}

////////////////////////////////
// GPIDescriptorHeapAllocator_DX12
////////////////////////////////
void GPIDescriptorHeapAllocator_DX12::Initialize( ID3D12Device* device )
{
	CreateDescriptorHeap( device, _heapContexts[ GPIResourceViewType_RTV ], 256, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE );
	CreateDescriptorHeap( device, _heapContexts[ GPIResourceViewType_DSV ], 128, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE );
	CreateDescriptorHeap( device, _heapContexts[ GPIResourceViewType_CBV ], 2048, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE );
	CreateDescriptorHeap( device, _heapContexts[ GPIResourceViewType_SRV ], 2048, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE );
	CreateDescriptorHeap( device, _heapContexts[ GPIResourceViewType_SRV_TEXTURE ], 2048, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE );
	CreateDescriptorHeap( device, _heapContexts[ GPIResourceViewType_UAV ], 1024, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE );
	CreateDescriptorHeap( device, _heapContexts[ GPIResourceViewType_UAV_SHADERHIDDEN ], 1024, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE );
	CreateDescriptorHeap( device, _heapContexts[ GPIResourceViewType_SAMPLER ], 256, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE );
}

GPIDescriptorHeapHandle_DX12 GPIDescriptorHeapAllocator_DX12::Allocate( const EGPIResourceViewType type )
{
	GPIDescriptorHeap_DX12& heapContext = _heapContexts[ type ];
	const GPIDescriptorHeapHandle_DX12 handle = heapContext.free.front();
	heapContext.used.emplace( handle );
	heapContext.free.pop();

	return handle;
}

void GPIDescriptorHeapAllocator_DX12::Release( const EGPIResourceViewType type, const GPIDescriptorHeapHandle_DX12& handle )
{
	GPIDescriptorHeap_DX12& heapContext = _heapContexts[ type ];
	heapContext.free.push( handle );
	heapContext.used.erase( handle );
}

ID3D12DescriptorHeap* GPIDescriptorHeapAllocator_DX12::GetHeap( const EGPIResourceViewType type )
{
	return _heapContexts[ type ].heap;
}
