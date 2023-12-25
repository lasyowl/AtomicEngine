#pragma once

#include "GPIResource.h"

struct ID3D12Resource;

struct VertexBuffer_DX12 : public IVertexBuffer
{
public:
	VertexBuffer_DX12( ID3D12Resource* resource, uint32 size, uint32 stride ) 
		: IVertexBuffer( size, stride )
		, _resource( resource ) {}

	virtual uint64 GetGPUVirtualAddress() const override;

	//virtual void Initialize( void* data, uint32 stride, uint32 size );
private:
	ID3D12Resource* _resource;
};

struct IndexBuffer_DX12 : public IIndexBuffer
{
public:
	IndexBuffer_DX12( ID3D12Resource* resource, uint32 size )
		: IIndexBuffer( size )
		, _resource( resource )
	{}

	virtual uint64 GetGPUVirtualAddress() const override;

	//virtual void Initialize( void* data, uint32 stride, uint32 size );
private:
	struct ID3D12Resource* _resource;
};