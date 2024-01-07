#pragma once

#include "EngineEssential.h"
#include "ECS_Component.h"
#include "ECS_System.h"
#include "Singleton.h"

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

	Entity CreateEntity()
	{
		assert( !_entities.empty() );

		Entity entity = _entities.front();
		_entities.pop();

		return entity;
	}

	void DestroyEntity( Entity entity )
	{
		_entities.push( entity );
	}

	template<typename T>
	void AddComponent( Entity entity )
	{
		std::unique_ptr<IComponentRegistry>& componentRegistry = _componentRegistry[ get_component_type_id<T>() ];
		if( !componentRegistry )
		{
			componentRegistry = std::make_unique<ComponentRegistry<T>>();
		}
		componentRegistry->AddComponent( entity );
	}

	template<typename T>
	void AddSystem()
	{
		std::unique_ptr<ISystem>& system = _systemRegistry[ get_system_type_id<T>() ];
		if( !system )
		{
			system = std::make_unique<T>();
		}
	}

	void RunSystems()
	{
		for( std::unique_ptr<ISystem>& system : _systemRegistry )
		{
			if( system )
			{
				system->RunSystem( _componentRegistry );
			}
		}
	}

private:
	std::queue<Entity> _entities;
	std::array<std::unique_ptr<IComponentRegistry>, NUM_COMPONENT_MAX> _componentRegistry;
	std::array<std::unique_ptr<ISystem>, NUM_SYSTEM_MAX> _systemRegistry;
};

///////////////////////
// Public functions
///////////////////////

Entity ECSCreateEntity();

template<typename T>
void ECSAddComponent( Entity entity )
{
	ECS::GetInstance().AddComponent<T>( entity );
}

template<typename T>
void ECSAddSystem()
{
	ECS::GetInstance().AddSystem<T>();
}

void ECSRunSystems();