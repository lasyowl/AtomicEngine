#include "stdafx.h"
#include "RenderResourceDX12.h"

#include <d3d12.h>

uint64 SVertexBufferContextDX12::GetGPUVirtualAddress()
{
	return Buffer->GetGPUVirtualAddress();
}

uint64 SIndexBufferContextDX12::GetGPUVirtualAddress()
{
	return Buffer->GetGPUVirtualAddress();
}