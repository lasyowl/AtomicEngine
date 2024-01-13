#pragma once

#include "ECS.h"
#include "Vector.h"
#include "GPIResource.h"

class LightSystem : public ISystem
{
public:
	DEFINE_ECS_SYSTEM( Light );
public:
	virtual void RunSystem( std::array<std::unique_ptr<IComponentRegistry>, NUM_COMPONENT_MAX>& componentRegistry ) override;
};
