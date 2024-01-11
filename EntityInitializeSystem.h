#pragma once

#include "ECS_System.h"

class EntityInitializeSystem : public ISystem
{
public:
	DEFINE_ECS_SYSTEM( EntityInitialize );
public:
	virtual void RunSystem( std::array<std::unique_ptr<struct IComponentRegistry>, NUM_COMPONENT_MAX>& componentRegistry ) override;
};
