#pragma once

#include "EngineEssential.h"

struct RawImage
{
	uint32 width;
	uint32 height;
	std::vector<uint8> data;
};