#pragma once

#include "ECS.h"
#include "Vector.h"

struct LightComponent
{
	DEFINE_ECS_COMPONENT( Light );

	float intensity;
};
