#include "stdafx.h"
#include "GPIResource_DX12.h"

#include <d3d12.h>

uint64 VertexBuffer_DX12::GetGPUVirtualAddress() const
{
	return _resource->GetGPUVirtualAddress();
}

uint64 IndexBuffer_DX12::GetGPUVirtualAddress() const
{
	return _resource->GetGPUVirtualAddress();
}