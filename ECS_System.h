#pragma once

#include "EngineEssential.h"

using SystemId = int32;

int32 systemTypeId = 0;
template<typename T> const int32 system_type_id = systemTypeId++;

class ISystem
{
public:
	template<typename T>
	ComponentRegistry<T>* GetRegistry( std::array<std::unique_ptr<struct IComponentRegistry>, NUM_COMPONENT_MAX>& componentRegistry )
	{
		std::unique_ptr<IComponentRegistry>& regInterface = componentRegistry[ component_type_id<T> ];
		return static_cast<ComponentRegistry<T>*>( regInterface.get() );
	}

	virtual void RunSystem( std::array<std::unique_ptr<struct IComponentRegistry>, NUM_COMPONENT_MAX>& componentRegistry ) abstract;
};

void ECSInitSystems()
{
	system_type_id<class CTestSystem>;
}