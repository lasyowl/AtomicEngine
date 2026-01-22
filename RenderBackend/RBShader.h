#pragma once

//enum EShaderType
//{
//	ShaderType_VertexShader = 0x01,
//	ShaderType_PixelShader = 0x02,
//	ShaderType_ComputeShader = 0x04,
//	ShaderType_RayTraceShader = 0x08,
//};
//
//constexpr bool IsValidShaderPermutation( EShaderType flags )
//{
//	/* Compute pipeline */
//	bool bGraphicsPipeline = flags | ShaderType_VertexShader;
//	bool bComputePipeline = flags | ShaderType_ComputeShader;
//	//bool bRayTracePipeline = flags | ShaderType_RayTraceShader;
//	
//	return bComputePipeline != bGraphicsPipeline;
//}

enum class EShaderType : uint8
{
	VertexShader,
	PixelShader,
	DomainShader,
	HullShader,
	GeometryShader,
	ComputeShader
};

enum class EShaderParameterType : uint8
{
    Unknown,
    Constant,
    ConstantBuffer,
    ShaderResource,
    UnorderedAccess,
    Sampler,
    DescriptorTable,
};

struct RBShaderParameter
{
    std::string name;
    EShaderParameterType type;

    uint8 registerIndex;
    uint8 spaceIndex;

    union
    {
        uint8 constantCount;
        uint8 resourceCount;
    };
};

struct RBShaderMacro
{
	std::string name;
	std::string value;
};

struct RBShaderDesc
{
    EShaderType type;

    std::string file;
    std::string entry;

    std::vector<RBShaderParameter> parameters;
    std::vector<RBShaderMacro> macros;
};
