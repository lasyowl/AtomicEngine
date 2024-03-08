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
	ERHIResourceFormat format;
	ERHIResourceDimension dimension;
};

struct RHIDepthStencilViewDesc
{
	ERHIResourceFormat format;
	ERHIResourceDimension dimension;
	ERHIDepthStencilViewFlag flag;
};

struct RHIConstantBufferViewDesc
{
	uint32 sizeInBytes;
};

struct RHIShaderResourceViewDesc
{
	ERHIResourceFormat format;
	ERHIResourceDimension dimension;
	uint32 numElements;
	uint32 byteStride;
};

struct RHIUnorderedAccessViewDesc
{
	ERHIResourceFormat format;
	ERHIResourceDimension dimension;
};

struct RHISamplerDesc
{

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
