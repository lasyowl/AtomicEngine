#include "stdafx.h"
#include "RenderResource.h"
#include "GraphicsInterface.h"

extern std::shared_ptr<IGraphicsInterface> gGraphics;

void CVertexBuffer::InitBuffer( void* Data, uint32 InStrideInByte, uint64 InSizeInByte )
{
	StrideInByte = InStrideInByte;
	SizeInByte = InSizeInByte;

	BufferContext = gGraphics->CreateVertexBuffer( Data, SizeInByte );
}

uint64 CVertexBuffer::GetGPUVirtualAddress() const
{
	return BufferContext->GetGPUVirtualAddress();
}

void CIndexBuffer::InitBuffer( void* Data, uint64 InSizeInByte )
{
	SizeInByte = InSizeInByte;

	BufferContext = gGraphics->CreateIndexBuffer( Data, SizeInByte );
}

uint64 CIndexBuffer::GetGPUVirtualAddress() const
{
	return BufferContext->GetGPUVirtualAddress();
}