#pragma once

#include "EngineEssential.h"
#include "ECS_Component.h"

using SystemId = int32;

static std::unordered_map<std::type_index, int> systemTypeIndexMap;
static int nextSystemIndex = 0;

template<typename T>
int get_system_type_id()
{
	std::type_index typeIndex = typeid( T );
	if( !systemTypeIndexMap.contains( typeIndex ) )
	{
		return systemTypeIndexMap[ typeIndex ] = nextSystemIndex++;
	}

	return systemTypeIndexMap[ typeIndex ];
}

class ISystem
{
public:
	template<typename T>
	ComponentRegistry<T>* GetRegistry( std::array<std::unique_ptr<struct IComponentRegistry>, NUM_COMPONENT_MAX>& componentRegistry )
	{
		std::unique_ptr<IComponentRegistry>& regInterface = componentRegistry[ get_component_type_id<T>() ];
		return static_cast<ComponentRegistry<T>*>( regInterface.get() );
	}

	virtual void RunSystem( std::array<std::unique_ptr<struct IComponentRegistry>, NUM_COMPONENT_MAX>& componentRegistry ) abstract;
};

void ECSInitSystems();