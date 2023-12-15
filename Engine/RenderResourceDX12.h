#pragma once

#include "RenderResource.h"

struct ID3D12Resource;

struct SVertexBufferContextDX12 : public IVertexBufferContext
{
public:
	SVertexBufferContextDX12( ID3D12Resource* InBuffer )
		: Buffer( InBuffer )
	{}

public:
	virtual uint64 GetGPUVirtualAddress() override;

private:
	ID3D12Resource* Buffer;
};

struct SIndexBufferContextDX12 : public IIndexBufferContext
{
public:
	SIndexBufferContextDX12( ID3D12Resource* InBuffer )
		: Buffer( InBuffer )
	{}

public:
	virtual uint64 GetGPUVirtualAddress() override;

private:
	ID3D12Resource* Buffer;
};