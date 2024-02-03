#include "EntityInitializeSystem.h"
#include "AssetLoader.h"
#include "MetaDataComponent.h"
#include "TransformComponent.h"
#include "PrimitiveComponent.h"
#include "LightComponent.h"
#include "SampleMesh.h"

void EntityInitializeSystem::RunSystem( std::array<std::unique_ptr<struct IComponentRegistry>, NUM_COMPONENT_MAX>& componentRegistry )
{
	ComponentRegistry<MetaDataComponent>* metaDataCompReg = GetRegistry<MetaDataComponent>( componentRegistry );
	ComponentRegistry<TransformComponent>* transformCompReg = GetRegistry<TransformComponent>( componentRegistry );
	ComponentRegistry<PrimitiveComponent>* primitiveCompReg = GetRegistry<PrimitiveComponent>( componentRegistry );
	ComponentRegistry<LightComponent>* lightCompReg = GetRegistry<LightComponent>( componentRegistry );
	if( !metaDataCompReg || !transformCompReg || !primitiveCompReg || !lightCompReg )
	{
		return;
	}

	for( Entity entity = 0; entity < NUM_ENTITY_MAX; ++entity )
	{
		if( !metaDataCompReg->HasComponent( entity ) )
		{
			continue;
		}

		MetaDataComponent& metaDataComp = metaDataCompReg->GetComponent( entity );
		if( metaDataComp.bLoaded )
		{
			continue;
		}

		if( lightCompReg->HasComponent( entity ) )
		{
			LightComponent& lightComp = lightCompReg->GetComponent( entity );
			if( metaDataComp.hash == 2 )
			{
				lightComp.intensity = 1.0f;
			}
		}

		metaDataComp.bLoaded = true;
	}
}