#pragma once

#include "EngineEssential.h"

/* Vertex buffer */
struct IVertexBuffer
{
protected:
	IVertexBuffer( uint32 size, uint32 stride ) : _size( size ), _stride( stride ) {}

public:
	virtual uint64 GetGPUVirtualAddress() const = 0;
	virtual uint64 GetSize() const { return _size; };
	virtual uint32 GetStride() const { return _stride; };

protected:
	uint32 _size;
	uint32 _stride;
};

/* Index buffer */
struct IIndexBuffer
{
protected:
	IIndexBuffer( uint32 size ) : _size( size ) {}

public:
	virtual uint64 GetGPUVirtualAddress() const = 0;
	virtual uint64 GetSize() const { return _size; };

protected:
	uint32 _size;
};

using IVertexBufferRef = std::shared_ptr<IVertexBuffer>;
using IIndexBufferRef = std::shared_ptr<IIndexBuffer>;