#include "LightSystem.h"
#include "AtomicEngine.h"
#include <RHI/RHI.h>
#include <Engine/TransformComponent.h>
#include <Engine/LightComponent.h>

void LightSystem::RunSystem( std::array<std::unique_ptr<IComponentRegistry>, NUM_COMPONENT_MAX>& componentRegistry )
{
	ComponentRegistry<TransformComponent>* transformCompReg = GetRegistry<TransformComponent>( componentRegistry );
	ComponentRegistry<LightComponent>* lightCompReg = GetRegistry<LightComponent>( componentRegistry );
	if( !lightCompReg )
	{
		return;
	}

	for( Entity entity = 0; entity < NUM_ENTITY_MAX; ++entity )
	{
		if( !lightCompReg->HasComponent( entity ) )
		{
			continue;
		}

		TransformComponent& transform = transformCompReg->GetComponent( entity );
		LightComponent& light = lightCompReg->GetComponent( entity );
		light.intensity = 1.0f;
	}
}