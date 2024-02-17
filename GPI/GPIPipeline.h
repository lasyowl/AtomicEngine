#pragma once

#include "GPIDefine.h"
#include "GPIShader.h"
#include "GPIResource.h"

enum EGPIPipelineType
{
	PipelineType_Graphics,
	PipelineType_Compute,
	PipelineType_RayTrace,
};

enum EGPIInputClass
{
	GPIInputClass_PerVertex,
	GPIInputClass_PerInstance
};

struct GPIPipelineInputDesc
{
	std::string semanticName;
	EGPIResourceFormat format;
	EGPIInputClass inputClass;
	uint32 inputSlot;
};

struct GPIPipelineInput
{
	std::vector<IGPIVertexBufferViewRef> vbv;
	std::vector<IGPIIndexBufferViewRef> ibv;
	IGPIRayTraceTopLevelASRef rtr;
};

struct GPIPipelineStateDesc
{
	uint32 id; // temp

	EGPIPipelineType pipelineType;

	bool bBindDepth;
	bool bEnableBlend;

	std::vector<GPIPipelineInputDesc> inputDesc;

	GPIShaderDesc vertexShader;
	GPIShaderDesc pixelShader;
	GPIShaderDesc computeShader;
	GPIShaderDesc raytraceShader;

	std::vector<EGPIResourceFormat> rtvFormats;

	uint32 numCBVs;
	uint32 numSRVs;
	uint32 numUAVs;
	std::vector<uint32> numTextures;
};

struct IGPIPipeline
{
};

typedef std::shared_ptr<IGPIPipeline> IGPIPipelineRef;