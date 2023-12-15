#pragma once

#include "EngineDefines.h"

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
