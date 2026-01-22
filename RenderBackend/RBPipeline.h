#pragma once

#include "RBDefine.h"
#include "RBShader.h"
#include "RBResource.h"

enum ERBPipelineType
{
	PipelineType_Graphics,
	PipelineType_Compute,
	PipelineType_RayTrace,
};

enum ERBInputClass
{
	RBInputClass_PerVertex,
	RBInputClass_PerInstance
};

struct RBPipelineInputDesc
{
	std::string semanticName;
	ERBResourceFormat format;
	ERBInputClass inputClass;
	uint8 inputSlot;
	uint8 byteOffset;
};

struct RBPipelineInput
{
	std::vector<IRBVertexBufferViewPtr> vbv;
	std::vector<IRBIndexBufferViewPtr> ibv;
	IRBRayTraceTopLevelASRef rtr;
};

struct RBPipelineStateDesc
{
	ERBPipelineType pipelineType;

	bool enableDepth;
	bool enableBlend;

	std::vector<RBPipelineInputDesc> inputDesc;

    IRBShaderPtr vertexShader;
    IRBShaderPtr pixelShader;
	RBShaderDesc* vertexShaderDesc;
	RBShaderDesc* pixelShaderDesc;
	uint32 pixelShaderParameterOffset;

	std::vector<ERBResourceFormat> rtvFormats;
};

struct IRBPipeline
{
};

using IRBPipelineRef = std::shared_ptr<IRBPipeline>;
