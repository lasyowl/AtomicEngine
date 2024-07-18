#pragma once

#include "ECSDefine.h"

struct MetaDataComponent : public IECSComponent
{
	DEFINE_ECS_COMPONENT( MetaData );

	uint64 hash;
	bool bLoaded;
};
