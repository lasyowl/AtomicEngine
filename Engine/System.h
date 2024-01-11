#pragma once

#include "EngineDefine.h"

enum
{
	Platform_Windows,
	Platform_Linux,
	Platform_Undefined
};

enum class EGraphicsAPI
{
	Windows,
	DX11,
	DX12
};

constexpr int GetPlatformType()
{
#if defined( _WIN32 ) || defined( _WIN64 )
	return Platform_Windows;
#elif defined( __linux__ )
	return Platform_Linux;
#else
	return Platform_Undefined;
#endif
}

class CSystemInfo
{
public:
	static int GetPlatformType();

	static void SetGraphicsAPIType( EGraphicsAPI InGraphicsAPIType ) { GraphicsAPIType = InGraphicsAPIType; }
	static EGraphicsAPI GetGraphicsAPIType() { return GraphicsAPIType; }

private:
	static EGraphicsAPI GraphicsAPIType;
};

struct SGraphicsAPICompatibility
{
	bool bDirectX8;
	bool bDirectX9;
	bool bDirectX10;
	bool bDirectX11;
	bool bDirectX12;
	bool bVulkan;
};

//SGraphicsAPICompatibility GetGraphicsAPICompatibility()
//{
//
//}
