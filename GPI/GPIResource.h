#pragma once

#include <Core/Vector.h>
#include <Core/Matrix.h>
#include "GPIDefine.h"

enum class EGPIResourceFormat;
enum class EGPIResourceDimension;

///////////////////////////////
// Resource Description
///////////////////////////////
struct GPIResourceClearValue
{
	EGPIResourceClearValueType type;
	union
	{
		Vec4 color;
		Vec2 depthStencil;
	};
};

struct GPIResourceDesc
{
	std::wstring name;

	EGPIResourceDimension dimension;
	EGPIResourceFormat format;

	uint64 width;
	uint32 height;
	uint16 depth;

	uint16 numMips;

	GPIResourceClearValue clearValue;

	EGPIResourceFlag flags;
	EGPIResourceStates initialState;
};

struct GPIRenderTargetViewDesc
{
	EGPIResourceFormat format;
	EGPIResourceDimension dimension;
};

struct GPIDepthStencilViewDesc
{
	EGPIResourceFormat format;
	EGPIResourceDimension dimension;
	EGPIDepthStencilViewFlag flag;
};

struct GPIConstantBufferViewDesc
{
	uint32 sizeInBytes;
};

struct GPIShaderResourceViewDesc
{
	EGPIResourceFormat format;
	EGPIResourceDimension dimension;
	uint32 numElements;
	uint32 byteStride;
};

struct GPIUnorderedAccessViewDesc
{
	EGPIResourceFormat format;
	EGPIResourceDimension dimension;
};

struct GPISamplerDesc
{

};

struct GPIRayTraceBottomLevelASDesc
{
	Mat4x4 transform;
};

struct GPIRayTraceTopLevelASDesc
{
};

///////////////////////////////
// Resource Interface
///////////////////////////////
struct IGPIResource
{
	virtual ~IGPIResource() {}

	EGPIResourceStates state;
};
using IGPIResourceRef = std::shared_ptr<IGPIResource>;

struct IGPIRenderTargetView
{
};

struct IGPIDepthStencilView
{

};

struct IGPIConstantBufferView
{

};

struct IGPIShaderResourceView
{

};

struct IGPIUnorderedAccessView
{

};

struct IGPITextureViewTable
{
	
};

struct IGPISampler
{

};

struct IGPIVertexBufferView
{
	uint32 size;
	uint32 stride;
};

struct IGPIIndexBufferView
{
	uint32 size;
};

struct IGPIRayTraceView
{
	uint32 byteSize;
};

struct IGPIRayTraceBottomLevelAS
{
	Mat4x4 transform;
};

struct IGPIRayTraceTopLevelAS
{
	IGPIResourceRef normalResource;
	IGPIResourceRef indexResource;
	IGPIResourceRef instanceContextResource;
	IGPIResourceRef materialResource;
};

using IGPIRenderTargetViewRef = std::shared_ptr<IGPIRenderTargetView>;
using IGPIDepthStencilViewRef = std::shared_ptr<IGPIDepthStencilView>;
using IGPIConstantBufferViewRef = std::shared_ptr<IGPIConstantBufferView>;
using IGPIShaderResourceViewRef = std::shared_ptr<IGPIShaderResourceView>;
using IGPIUnorderedAccessViewRef = std::shared_ptr<IGPIUnorderedAccessView>;
using IGPITextureViewTableRef = std::shared_ptr<IGPITextureViewTable>;
using IGPISamplerRef = std::shared_ptr<IGPISampler>;
using IGPIVertexBufferViewRef = std::shared_ptr<IGPIVertexBufferView>;
using IGPIIndexBufferViewRef = std::shared_ptr<IGPIIndexBufferView>;
using IGPIRayTraceViewRef = std::shared_ptr<IGPIRayTraceView>;
using IGPIRayTraceBottomLevelASRef = std::shared_ptr<IGPIRayTraceBottomLevelAS>;
using IGPIRayTraceTopLevelASRef = std::shared_ptr<IGPIRayTraceTopLevelAS>;
