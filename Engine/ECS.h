#pragma once

#include "ECSDefine.h"
#include "ECS_Component.h"
#include "ECS_System.h"
#include "Singleton.h"
#include <Engine/ArcheType.h>

///////////////////////
// ECS
///////////////////////
class ECS : public TSingleton<ECS>
{
public:
	ECS()
	{
		for( Entity entity = 0; entity < NUM_ENTITY_MAX; ++entity )
		{
			_entities.push( entity );
		}

		ECSInitSystems();
	}

	Entity CreateEntity();
	Entity CreateEntityWithMetaData( uint64 metaDataHash );

	void DestroyEntity( Entity entity ) { _entities.push( entity ); }

	uint32 GetComponentSize( ECSComponentType type ) { return _componentRegistry[ type ]->GetComponentSize(); }

	template<typename T>
	void AddComponent( Entity entity, const T* data )
	{
		std::unique_ptr<IComponentRegistry>& componentRegistry = _componentRegistry[ T::type ];
		if( !componentRegistry )
		{
			componentRegistry = std::make_unique<ComponentRegistry<T>>();
		}
		componentRegistry->AddComponent( entity, data );
	}

	template<typename T>
	void AddSystem()
	{
		std::unique_ptr<ISystem>& system = _systemRegistry[ T::type ];
		if( !system )
		{
			system = std::make_unique<T>();
		}
	}

	ArcheTypeRegistry& GetArcheTypeRegistry()
	{
		return _archeTypeRegistry;
	}

	void RunSystems();

	void ExportEntity( const Entity entity, const std::string& fileName );
	Entity ImportEntity( const std::string& fileName );

	void LoadSceneData();

private:
	std::queue<Entity> _entities;
	std::array<std::unique_ptr<IComponentRegistry>, NUM_COMPONENT_MAX> _componentRegistry;
	std::array<std::unique_ptr<ISystem>, NUM_SYSTEM_MAX> _systemRegistry;

	ArcheTypeRegistry _archeTypeRegistry;
};

///////////////////////
// Public functions
///////////////////////

Entity ECSCreateEntity();
Entity ECSCreateEntityWithMetaData( uint64 assetHash );

template<typename T>
void ECSAddComponent( Entity entity, const T* data )
{
	ECS::GetInstance().AddComponent<T>( entity, data );
}

template<typename T>
void ECSAddSystem()
{
	ECS::GetInstance().AddSystem<T>();
}

ArcheTypeRegistry& ECSGetArcheTypeRegistry();

void ECSRunSystems();