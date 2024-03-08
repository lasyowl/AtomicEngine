#pragma once

#include "ECS.h"
#include <Core/Vector.h>
#include <RHI/RHIResource.h>

class LightSystem : public ISystem
{
public:
	DEFINE_ECS_SYSTEM( Light );
public:
	virtual void RunSystem( std::array<std::unique_ptr<IComponentRegistry>, NUM_COMPONENT_MAX>& componentRegistry ) override;
};
