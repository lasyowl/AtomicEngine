#include "stdafx.h"
#include "ECS.h"
#include "MetaDataComponent.h"

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
	AddComponent<MetaDataComponent>( entity );

	std::unique_ptr<IComponentRegistry>& regInterface = _componentRegistry[ MetaDataComponent::type ];
	ComponentRegistry<MetaDataComponent>* registry = static_cast<ComponentRegistry<MetaDataComponent>*>( regInterface.get() );
	
	MetaDataComponent& component = registry->GetComponent( entity );
	component.hash = metaDataHash;

	return entity;
}