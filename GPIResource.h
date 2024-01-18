#pragma once

#include "EngineEssential.h"
#include "GPIDefine.h"
#include "Vector.h"

enum EGPIResourceFlag;
enum EGPIResourceUsage;
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
	EGPIResourceDimension dimension;
	EGPIResourceFormat format;

	uint64 width;
	uint32 height;
	uint16 depth;

	uint16 numMips;

	GPIResourceClearValue clearValue;

	EGPIResourceFlag flags;
	EGPIResourceUsage usage;
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

};

struct GPIUnorderedAccessViewDesc
{

};

struct GPISamplerDesc
{

};

///////////////////////////////
// Resource Interface
///////////////////////////////
struct IGPIResource
{

};

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

using IGPIResourceRef = std::shared_ptr<IGPIResource>;
using IGPIRenderTargetViewRef = std::shared_ptr<IGPIRenderTargetView>;
using IGPIDepthStencilViewRef = std::shared_ptr<IGPIDepthStencilView>;
using IGPIConstantBufferViewRef = std::shared_ptr<IGPIConstantBufferView>;
using IGPIShaderResourceViewRef = std::shared_ptr<IGPIShaderResourceView>;
using IGPIUnorderedAccessViewRef = std::shared_ptr<IGPIUnorderedAccessView>;
using IGPISamplerRef = std::shared_ptr<IGPISampler>;
using IGPIVertexBufferViewRef = std::shared_ptr<IGPIVertexBufferView>;
using IGPIIndexBufferViewRef = std::shared_ptr<IGPIIndexBufferView>;
