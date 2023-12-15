#pragma once

#include "EngineEssential.h"

#include <d3d12.h>

namespace Predefined
{
D3D12_RESOURCE_DESC ResourceDescBuffer( uint64 Width, uint32 Height = 1, uint16 Depth = 1 )
{
	D3D12_RESOURCE_DESC BufferDesc{};
	BufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	BufferDesc.Alignment = 0;
	BufferDesc.Width = Width;
	BufferDesc.Height = Height;
	BufferDesc.DepthOrArraySize = Depth;
	BufferDesc.MipLevels = 1;
	BufferDesc.Format = DXGI_FORMAT_UNKNOWN;
	BufferDesc.SampleDesc.Count = 1;
	BufferDesc.SampleDesc.Quality = 0;
	BufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	BufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	return BufferDesc;
}

constexpr D3D12_HEAP_PROPERTIES HeapProperties( D3D12_HEAP_TYPE HeapType )
{
	D3D12_HEAP_PROPERTIES HeapProp{};
	HeapProp.Type = HeapType;
	HeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	HeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	HeapProp.CreationNodeMask = 1;
	HeapProp.VisibleNodeMask = 1;

	return HeapProp;
}
}