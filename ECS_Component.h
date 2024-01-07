#pragma once

#include "EngineEssential.h"

using ComponentId = int32;

static std::unordered_map<std::type_index, int> typeIndexMap;
static int nextIndex = 0;

template<typename T>
int get_component_type_id()
{
	std::type_index typeIndex = typeid( T );
	if( !typeIndexMap.contains( typeIndex ) )
	{
		return typeIndexMap[ typeIndex ] = nextIndex++;
	}

	return typeIndexMap[ typeIndex ];
}

///////////////////////
// Component
///////////////////////
struct IComponentRegistry
{
	virtual ~IComponentRegistry() {}

	virtual void AddComponent( Entity entity ) abstract;
};

template <typename T>
struct ComponentRegistry : public IComponentRegistry
{
	virtual void AddComponent( Entity entity ) override
	{
		_registry[ entity ] = {};

		_signature[ entity ] = true;
	}

	bool HasComponent( Entity entity )
	{
		return _signature[ entity ];
	}

	T& GetComponent( Entity entity )
	{
		return _registry[ entity ];
	}

private:
	std::array<T, NUM_ENTITY_MAX> _registry;
	std::bitset<NUM_ENTITY_MAX> _signature;
};
