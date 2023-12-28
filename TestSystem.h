#pragma once

#include "ECS.h"

struct TestComponent
{
	int32 x;
};

class CTestSystem : public ISystem
{
public:
	virtual void RunSystem( std::array<std::unique_ptr<struct IComponentRegistry>, NUM_COMPONENT_MAX>& componentRegistry ) override
	{
		std::unique_ptr<IComponentRegistry>& testCompRegInterface = componentRegistry[ component_type_id<TestComponent> ];
		ComponentRegistry<TestComponent>* testCompReg = static_cast<ComponentRegistry<TestComponent>*>(testCompRegInterface.get());

		for( Entity entity = 0; entity < NUM_ENTITY_MAX; ++entity )
		{
			if( !testCompReg->HasComponent( entity ) )
			{
				continue;
			}

			TestComponent& component = testCompReg->GetComponent( entity );
			component.x = 1;
		}
	}
};