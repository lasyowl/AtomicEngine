#pragma once

#include "EngineDefines.h"

struct SRect
{
	int32 Left;
	int32 Right;
	int32 Top;
	int32 Bottom;

	int32 GetWidth() { return Right - Left; }
	int32 GetHeight() { return Top - Bottom; }
};