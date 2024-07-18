#pragma once

#include "ECS.h"
#include <Core/Vector.h>

struct TransformComponent : public IECSComponent
{
	DEFINE_ECS_COMPONENT( Transform );

	Vec3 position;
	Vec3 rotation;
	Vec3 scale;
};
