#pragma once

#include "EngineEssential.h"

struct IVertexBufferContext
{
public:
	virtual uint64 GetGPUVirtualAddress() abstract;
};

struct IIndexBufferContext
{
	virtual uint64 GetGPUVirtualAddress() abstract;
};

typedef std::shared_ptr<IVertexBufferContext> IVertexBufferContextRef;
typedef std::shared_ptr<IIndexBufferContext> IIndexBufferContextRef;

/* Vertex buffer */
struct CVertexBuffer
{
public:
	void InitBuffer( void* Data, uint32 InStrideInByte, uint64 InSizeInByte );

	uint64 GetGPUVirtualAddress() const;
	uint64 GetSizeInByte() const { return SizeInByte; }
	uint32 GetStrideInByte() const { return StrideInByte; }

private:
	IVertexBufferContextRef BufferContext;

	uint64 SizeInByte;
	uint32 StrideInByte;
};

/* Index buffer */
struct CIndexBuffer
{
public:
	void InitBuffer( void* Data, uint64 InSizeInByte );

	uint64 GetGPUVirtualAddress() const;
	uint64 GetSizeInByte() const { return SizeInByte; }

private:
	IIndexBufferContextRef BufferContext;

	uint64 SizeInByte;
};