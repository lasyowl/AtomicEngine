#pragma once

#include "EngineEssential.h"

enum EShaderType
{
	ShaderType_VertexShader = 0x01,
	ShaderType_PixelShader = 0x02,
	ShaderType_ComputeShader = 0x04,
};

constexpr bool IsValidShaderPermutation( EShaderType flags )
{
	/* Compute pipeline */
	bool bGraphicsPipeline = flags | ShaderType_VertexShader;
	bool bComputePipeline = flags | ShaderType_ComputeShader;
	
	return bComputePipeline != bGraphicsPipeline;
}

struct GPIShaderMacro
{
	std::string name;
	std::string value;
};

struct GPIShaderDesc
{
	uint32 hash; // temp
	EShaderType type;
	std::string file;
	std::string entry;
	std::vector<GPIShaderMacro> macros;
};

struct IGPIShader
{

};