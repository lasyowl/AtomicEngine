#pragma once

#include "EngineEssential.h"

enum class EShaderType : uint8
{
	VS,
	PS,
	DS,
	HS,
	GS,
	CS
};

class CShader
{
	std::wstring FileName;
	// std::string bytecode;
	EShaderType ShaderType;
};

class CShaderSet
{
	std::vector<CShader> Shaders;
};