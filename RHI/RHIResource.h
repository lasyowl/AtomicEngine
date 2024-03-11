#pragma once

#include <Core/Vector.h>
#include <Core/Matrix.h>
#include "RHIDefine.h"

enum class ERHIResourceFormat;
enum class ERHIResourceDimension;

///////////////////////////////
// Resource Description
///////////////////////////////
struct RHIResourceClearValue
{
	ERHIResourceClearValueType type;
	union
	{
		Vec4 color;
		Vec2 depthStencil;
	};
};

struct RHIResourceDesc
{
	std::wstring name;

	ERHIResourceDimension dimension;
	ERHIResourceFormat format;

	uint64 width;
	uint32 height;
	uint16 depth;

	uint16 numMips;

	RHIResourceClearValue clearValue;

	ERHIResourceFlag flags;
	ERHIResourceStates initialState;
};

struct RHIRenderTargetViewDesc
{
public:
	RHIRenderTargetViewDesc()
		: bStatic( false )
		, format( ERHIResourceFormat::Unknown )
		, dimension( ERHIResourceDimension::Unknown )
	{}
	RHIRenderTargetViewDesc( bool bInStatic, ERHIResourceFormat inFormat, ERHIResourceDimension inDimension )
		: bStatic( bInStatic )
		, format( inFormat )
		, dimension( inDimension )
	{}

public:
	bool bStatic;
	ERHIResourceFormat format;
	ERHIResourceDimension dimension;
};

struct RHIDepthStencilViewDesc
{
public:
	RHIDepthStencilViewDesc()
		: bStatic( false )
		, format( ERHIResourceFormat::Unknown )
		, dimension( ERHIResourceDimension::Unknown )
		, flag( RHIDepthStencilViewFlag_None )
	{}
	RHIDepthStencilViewDesc( bool bInStatic, ERHIResourceFormat inFormat, ERHIResourceDimension inDimension, ERHIDepthStencilViewFlag inFlag )
		: bStatic( bInStatic )
		, format( inFormat )
		, dimension( inDimension )
		, flag( inFlag )
	{}

public:
	bool bStatic;
	ERHIResourceFormat format;
	ERHIResourceDimension dimension;
	ERHIDepthStencilViewFlag flag;
};

struct RHIConstantBufferViewDesc
{
public:
	RHIConstantBufferViewDesc()
		: bStatic( false )
		, sizeInBytes( 0 )
	{}
	RHIConstantBufferViewDesc( bool bInStatic, uint32 inSizeInBytes )
		: bStatic( bInStatic )
		, sizeInBytes( inSizeInBytes )
	{}

public:
	bool bStatic;
	uint32 sizeInBytes;
};

struct RHIShaderResourceViewDesc
{
public:
	RHIShaderResourceViewDesc()
		: bStatic( false )
		, format( ERHIResourceFormat::Unknown )
		, dimension( ERHIResourceDimension::Unknown )
		, numElements( 0 )
		, byteStride( 0 )
	{}
	RHIShaderResourceViewDesc( bool bInStatic, ERHIResourceFormat inFormat, ERHIResourceDimension inDimension, uint32 inNumElements, uint32 inByteStride )
		: bStatic( bInStatic )
		, format( inFormat )
		, dimension( inDimension )
		, numElements( inNumElements )
		, byteStride( inByteStride )
	{}

public:
	bool bStatic;
	ERHIResourceFormat format;
	ERHIResourceDimension dimension;
	uint32 numElements;
	uint32 byteStride;
};

struct RHIUnorderedAccessViewDesc
{
public:
	RHIUnorderedAccessViewDesc()
		: bStatic( false )
		, format( ERHIResourceFormat::Unknown )
		, dimension( ERHIResourceDimension::Unknown )
	{}
	RHIUnorderedAccessViewDesc( bool bInStatic, ERHIResourceFormat inFormat, ERHIResourceDimension inDimension )
		: bStatic( bInStatic )
		, format( inFormat )
		, dimension( inDimension )
	{}

public:
	bool bStatic;
	ERHIResourceFormat format;
	ERHIResourceDimension dimension;
};

struct RHISamplerDesc
{
public:
	RHISamplerDesc() : bStatic( false ) {}

public:
	bool bStatic;
};

struct RHIRayTraceBottomLevelASDesc
{
	Mat4x4 transform;
};

struct RHIRayTraceTopLevelASDesc
{
};

///////////////////////////////
// Resource Interface
///////////////////////////////
struct IRHIResource
{
	virtual ~IRHIResource() {}

	ERHIResourceStates state;
};
using IRHIResourceRef = std::shared_ptr<IRHIResource>;

struct IRHIRenderTargetView
{
};

struct IRHIDepthStencilView
{

};

struct IRHIConstantBufferView
{

};

struct IRHIShaderResourceView
{

};

struct IRHIUnorderedAccessView
{

};

struct IRHITextureViewTable
{
	
};

struct IRHIDescriptorTableView
{

};

struct IRHISampler
{

};

struct IRHIVertexBufferView
{
	uint32 size;
	uint32 stride;
};

struct IRHIIndexBufferView
{
	uint32 size;
};

struct IRHIRayTraceView
{
	uint32 byteSize;
};

struct IRHIRayTraceBottomLevelAS
{
	Mat4x4 transform;
};

struct IRHIRayTraceTopLevelAS
{
	IRHIResourceRef normalResource;
	IRHIResourceRef indexResource;
	IRHIResourceRef instanceContextResource;
	IRHIResourceRef materialResource;
};

using IRHIRenderTargetViewRef = std::shared_ptr<IRHIRenderTargetView>;
using IRHIDepthStencilViewRef = std::shared_ptr<IRHIDepthStencilView>;
using IRHIConstantBufferViewRef = std::shared_ptr<IRHIConstantBufferView>;
using IRHIShaderResourceViewRef = std::shared_ptr<IRHIShaderResourceView>;
using IRHIUnorderedAccessViewRef = std::shared_ptr<IRHIUnorderedAccessView>;
using IRHITextureViewTableRef = std::shared_ptr<IRHITextureViewTable>;
using IRHIDescriptorTableViewRef = std::shared_ptr<IRHIDescriptorTableView>;
using IRHISamplerRef = std::shared_ptr<IRHISampler>;
using IRHIVertexBufferViewRef = std::shared_ptr<IRHIVertexBufferView>;
using IRHIIndexBufferViewRef = std::shared_ptr<IRHIIndexBufferView>;
using IRHIRayTraceViewRef = std::shared_ptr<IRHIRayTraceView>;
using IRHIRayTraceBottomLevelASRef = std::shared_ptr<IRHIRayTraceBottomLevelAS>;
using IRHIRayTraceTopLevelASRef = std::shared_ptr<IRHIRayTraceTopLevelAS>;
