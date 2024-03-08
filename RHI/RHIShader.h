#pragma once

enum EShaderType
{
	ShaderType_VertexShader = 0x01,
	ShaderType_PixelShader = 0x02,
	ShaderType_ComputeShader = 0x04,
	ShaderType_RayTraceShader = 0x08,
};

constexpr bool IsValidShaderPermutation( EShaderType flags )
{
	/* Compute pipeline */
	bool bGraphicsPipeline = flags | ShaderType_VertexShader;
	bool bComputePipeline = flags | ShaderType_ComputeShader;
	//bool bRayTracePipeline = flags | ShaderType_RayTraceShader;
	
	return bComputePipeline != bGraphicsPipeline;
}

struct RHIShaderMacro
{
	std::string name;
	std::string value;
};

struct RHIShaderDesc
{
	uint32 hash; // temp
	EShaderType type;
	std::string file;
	std::string entry;
	std::vector<RHIShaderMacro> macros;
};

struct IRHIShader
{
};