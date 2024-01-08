#pragma once

#include "EngineEssential.h"
#include "GPIShader.h"

enum EGPIPipelineType
{
	PipelineType_Graphics,
	PipelineType_Compute,
};

enum EGPIDataFormat
{
	GPIDataFormat_Unknown,
	GPIDataFormat_B8G8R8A8,
	GPIDataFormat_R32G32_Float,
	GPIDataFormat_R32G32B32_Float,
};

enum EGPIInputClass
{
	GPIInputClass_PerVertex,
	GPIInputClass_PerInstance
};

struct GPIPipelineInputDesc
{
	std::string semanticName;
	EGPIDataFormat format;
	EGPIInputClass inputClass;
	uint32 inputSlot;
};

struct GPIPipelineStateDesc
{
	uint32 hash; // temp

	bool bRenderSwapChainBuffer;
	uint32 numRenderTargets;

	EGPIPipelineType pipelineType;

	std::vector<GPIPipelineInputDesc> inputDesc;

	GPIShaderDesc vertexShader;
	GPIShaderDesc pixelShader;
	GPIShaderDesc computeShader;
};

struct IGPIPipeline
{
	IGPIShader vertexShader;
	IGPIShader pixelShader;
	IGPIShader computeShader;
};