#pragma once

#include "ECS.h"
#include <Core/Vector.h>

struct LightComponent
{
	DEFINE_ECS_COMPONENT( Light );

	float intensity;
};
