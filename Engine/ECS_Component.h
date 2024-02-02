#pragma once

#include "ECSDefine.h"

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
	ComponentRegistry()
	{
		memset( _registry.data(), 0, _registry.size() * sizeof( T ) );
	}

	virtual void AddComponent( Entity entity ) override
	{
		//_registry[ entity ] = {};

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
