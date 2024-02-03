#include "ECS.h"
#include "MetaDataComponent.h"
#include <Engine/Scene.h>

Entity ECSCreateEntity()
{
	return ECS::GetInstance().CreateEntity();
}

Entity ECSCreateEntityWithMetaData( uint64 assetHash )
{
	return ECS::GetInstance().CreateEntityWithMetaData( assetHash );
}

void ECSRunSystems()
{
	ECS::GetInstance().RunSystems();
}

Entity ECS::CreateEntity()
{
	assert( !_entities.empty() );

	Entity entity = _entities.front();
	_entities.pop();

	return entity;
}

Entity ECS::CreateEntityWithMetaData( uint64 metaDataHash )
{
	Entity entity = CreateEntity();
	AddComponent<MetaDataComponent>( entity, nullptr );

	std::unique_ptr<IComponentRegistry>& regInterface = _componentRegistry[ MetaDataComponent::type ];
	ComponentRegistry<MetaDataComponent>* registry = static_cast<ComponentRegistry<MetaDataComponent>*>( regInterface.get() );
	
	MetaDataComponent& component = registry->GetComponent( entity );
	component.hash = metaDataHash;

	return entity;
}

void ECS::RunSystems()
{
	for( std::unique_ptr<ISystem>& system : _systemRegistry )
	{
		if( system )
		{
			system->RunSystem( _componentRegistry );
		}
	}
}

void ECS::LoadSceneData()
{
	/*SceneData scene;
	for( int32 index = 0; index < NUM_ENTITY_MAX; ++index )
	{
		if( scene.signature[ index ] )
		{
			
		}
	}*/
}