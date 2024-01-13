#pragma once

#include "EngineEssential.h"
#include "GPIShader.h"

enum EGPIPipelineType
{
	PipelineType_Graphics,
	PipelineType_Compute,
};

enum EGPIBufferFormat
{
	GPIBufferFormat_Unknown,
	GPIBufferFormat_B8G8R8A8,
	GPIBufferFormat_B8G8R8A8_SRGB,
	GPIBufferFormat_R32G32_Float,
	GPIBufferFormat_R32G32B32_Float,
};

enum EGPIInputClass
{
	GPIInputClass_PerVertex,
	GPIInputClass_PerInstance
};

struct GPIPipelineInputDesc
{
	std::string semanticName;
	EGPIBufferFormat format;
	EGPIInputClass inputClass;
	uint32 inputSlot;
};

struct GPIRenderTargetDesc
{
	EGPIBufferFormat format;
};

struct GPIPipelineStateDesc
{
	uint32 hash; // temp

	EGPIPipelineType pipelineType;

	bool bRenderSwapChainBuffer;

	bool bWriteDepth;

	uint32 numRenderTargets;
	std::vector<GPIRenderTargetDesc> renderTargetDesc;

	std::vector<GPIPipelineInputDesc> inputDesc;

	GPIShaderDesc vertexShader;
	GPIShaderDesc pixelShader;
	GPIShaderDesc computeShader;

	uint32 numConstantBuffers;
	uint32 constBufferSize;

	uint32 numResources;
	std::vector<uint32> resourceBufferSizes;
};

struct IGPIPipeline
{
	IGPIShader vertexShader;
	IGPIShader pixelShader;
	IGPIShader computeShader;
};