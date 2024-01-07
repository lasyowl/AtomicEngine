#pragma once

#include "ECS.h"
#include "Vector.h"
#include "Matrix.h"

struct SceneViewComponent
{
	Vec3 position;
	Vec3 direction;

	Mat4x4 matView;
	Mat4x4 matProjection;
};

class SceneViewSystem : public ISystem
{
public:
	virtual void RunSystem( std::array<std::unique_ptr<struct IComponentRegistry>, NUM_COMPONENT_MAX>& componentRegistry ) override;
};
