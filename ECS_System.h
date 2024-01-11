#pragma once

#include "EngineEssential.h"
#include "ECSDefine.h"
#include "ECS_Component.h"

class ISystem
{
public:
	virtual ~ISystem() {}

public:
	template<typename T>
	ComponentRegistry<T>* GetRegistry( std::array<std::unique_ptr<struct IComponentRegistry>, NUM_COMPONENT_MAX>& componentRegistry )
	{
		std::unique_ptr<IComponentRegistry>& regInterface = componentRegistry[ T::type ];
		return static_cast<ComponentRegistry<T>*>( regInterface.get() );
	}

	virtual void RunSystem( std::array<std::unique_ptr<struct IComponentRegistry>, NUM_COMPONENT_MAX>& componentRegistry ) abstract;
};

void ECSInitSystems();