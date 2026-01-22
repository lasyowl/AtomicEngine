#pragma once

#include <Core/Vector.h>
#include <Core/Matrix.h>
#include "RBDefine.h"

enum class ERBResourceFormat;
enum class ERBResourceDimension;

///////////////////////////////
// Resource Description
///////////////////////////////
struct RBResourceClearValue
{
	ERBResourceClearValueType type;
	union
	{
		Vec4 color;
		Vec2 depthStencil;
	};
};

struct RBResourceDesc
{
	std::string name;

	ERBResourceDimension dimension;
	ERBResourceFormat format;

	uint64 width;
	uint32 height;
	uint16 depth;

	uint16 numMips;

	RBResourceClearValue clearValue;

	ERBResourceFlag flags;
	ERBResourceStates initialState;
};

struct RBRenderTargetViewDesc
{
	bool bStatic = false;
	ERBResourceFormat format = ERBResourceFormat::Unknown;
	ERBResourceDimension dimension = ERBResourceDimension::Unknown;
};

struct RBDepthStencilViewDesc
{
	bool bStatic = false;
	ERBResourceFormat format = ERBResourceFormat::Unknown;
	ERBResourceDimension dimension = ERBResourceDimension::Unknown;
	ERBDepthStencilViewFlag flag = RBDepthStencilViewFlag_None;
};

struct RBConstantBufferViewDesc
{
	bool bStatic = false;
	uint32 sizeInBytes = 0;
};

struct RBShaderResourceViewDesc
{
	bool bStatic = false;
	ERBResourceFormat format = ERBResourceFormat::Unknown;
	ERBResourceDimension dimension = ERBResourceDimension::Unknown;
	uint32 numElements = 0;
	uint32 byteStride = 0;
};

struct RBUnorderedAccessViewDesc
{
	bool bStatic = false;
	ERBResourceFormat format = ERBResourceFormat::Unknown;
	ERBResourceDimension dimension = ERBResourceDimension::Unknown;
};

struct RBSamplerDesc
{
	bool bStatic = false;
};

struct RBRayTraceBottomLevelASDesc
{
	Mat4x4 transform;
};

struct RBRayTraceTopLevelASDesc
{
};

///////////////////////////////
// Resource Interface
///////////////////////////////
struct IRBResource
{
	ERBResourceStates state = RBResourceState_Common;
};
using IRBResourcePtr = std::shared_ptr<IRBResource>;

struct IRBShader
{

};
using IRBShaderPtr = std::shared_ptr<IRBShader>;

struct IRBSwapChain
{

};

struct IRBRenderTargetView
{

};

struct IRBDepthStencilView
{

};

struct IRBConstantBufferView
{

};

struct IRBShaderResourceView
{

};

struct IRBUnorderedAccessView
{

};

struct IRBTextureViewTable
{
	
};

struct IRBDescriptorTableView
{

};

struct IRBSampler
{

};

struct IRBVertexBufferView
{

};

struct IRBIndexBufferView
{

};

struct IRBRayTraceView
{
	uint32 byteSize;
};

struct IRBRayTraceBottomLevelAS
{
	Mat4x4 transform;
};

struct IRBRayTraceTopLevelAS
{
	IRBResourcePtr normalResource;
	IRBResourcePtr indexResource;
	IRBResourcePtr instanceContextResource;
	IRBResourcePtr materialResource;
};

using IRBSwapChainRef = std::shared_ptr<IRBSwapChain>;
using IRBRenderTargetViewPtr = std::shared_ptr<IRBRenderTargetView>;
using IRBDepthStencilViewRef = std::shared_ptr<IRBDepthStencilView>;
using IRBConstantBufferViewPtr = std::shared_ptr<IRBConstantBufferView>;
using IRBShaderResourceViewPtr = std::shared_ptr<IRBShaderResourceView>;
using IRBUnorderedAccessViewPtr = std::shared_ptr<IRBUnorderedAccessView>;
using IRBTextureViewTablePtr = std::shared_ptr<IRBTextureViewTable>;
using IRBDescriptorTableViewPtr = std::shared_ptr<IRBDescriptorTableView>;
using IRBSamplerPtr = std::shared_ptr<IRBSampler>;
using IRBVertexBufferViewPtr = std::shared_ptr<IRBVertexBufferView>;
using IRBIndexBufferViewPtr = std::shared_ptr<IRBIndexBufferView>;
using IRBRayTraceViewRef = std::shared_ptr<IRBRayTraceView>;
using IRBRayTraceBottomLevelASRef = std::shared_ptr<IRBRayTraceBottomLevelAS>;
using IRBRayTraceTopLevelASRef = std::shared_ptr<IRBRayTraceTopLevelAS>;
