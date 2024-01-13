#pragma once

#include "ECS.h"
#include "Vector.h"

struct TransformComponent
{
	DEFINE_ECS_COMPONENT( Transform );

	Vec3 position;
	Vec3 rotation;
	Vec3 scale;
};
