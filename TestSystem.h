#pragma once

#include "ECS.h"

struct STestComponent
{
	int32 x;
};

class CTestSystem : public ISystem
{
public:
	virtual void RunSystem( std::array<std::unique_ptr<struct IComponentRegistry>, NUM_COMPONENT_MAX>& componentRegistry ) override
	{
		std::unique_ptr<IComponentRegistry>& testCompRegInterface = componentRegistry[ component_type_id<STestComponent> ];
		SComponentRegistry<STestComponent>* testCompReg = static_cast<SComponentRegistry<STestComponent>*>(testCompRegInterface.get());

		for( Entity entity = 0; entity < NUM_ENTITY_MAX; ++entity )
		{
			if( !testCompReg->HasComponent( entity ) )
			{
				continue;
			}

			STestComponent& component = testCompReg->GetComponent( entity );
			component.x = 1;
		}
	}
};