#pragma once

#include "ECSDefine.h"

struct MetaDataComponent
{
	DEFINE_ECS_COMPONENT( MetaData );

	uint64 hash;
	bool bLoaded;
};
