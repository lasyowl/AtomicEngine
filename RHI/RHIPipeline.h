#pragma once

#include "RHIDefine.h"
#include "RHIShader.h"
#include "RHIResource.h"

enum ERHIPipelineType
{
	PipelineType_Graphics,
	PipelineType_Compute,
	PipelineType_RayTrace,
};

enum ERHIInputClass
{
	RHIInputClass_PerVertex,
	RHIInputClass_PerInstance
};

struct RHIPipelineInputDesc
{
	std::string semanticName;
	ERHIResourceFormat format;
	ERHIInputClass inputClass;
	uint32 inputSlot;
};

struct RHIPipelineInput
{
	std::vector<IRHIVertexBufferViewRef> vbv;
	std::vector<IRHIIndexBufferViewRef> ibv;
	IRHIRayTraceTopLevelASRef rtr;
};

struct RHIPipelineStateDesc
{
	uint32 id; // temp

	ERHIPipelineType pipelineType;

	bool bBindDepth;
	bool bEnableBlend;

	std::vector<RHIPipelineInputDesc> inputDesc;

	RHIShaderDesc vertexShader;
	RHIShaderDesc pixelShader;
	RHIShaderDesc computeShader;
	RHIShaderDesc raytraceShader;

	std::vector<ERHIResourceFormat> rtvFormats;

	uint32 numCBVs;
	uint32 numSRVs;
	uint32 numUAVs;
	std::vector<uint32> numTextures;
};

struct IRHIPipeline
{
};

typedef std::shared_ptr<IRHIPipeline> IRHIPipelineRef;