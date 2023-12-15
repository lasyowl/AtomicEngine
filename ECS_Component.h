#pragma once

#include "EngineDefines.h"

using ComponentId = int32;

int32 componentTypeId = 0;
template<typename T> const int32 component_type_id = componentTypeId++;

///////////////////////
// Component
///////////////////////
using Signature = std::bitset<NUM_COMPONENT_MAX>;

struct IComponentRegistry
{
	virtual ~IComponentRegistry() {}

	virtual void AddComponent( Entity entity ) = 0;
};

template <typename T>
struct SComponentRegistry : public IComponentRegistry
{
	virtual void AddComponent( Entity entity ) override
	{
		_registry[ entity ] = {};
	}

	T& GetComponent( Entity entity )
	{
		return _registry[ entity ];
	}

private:
	std::array<T, NUM_ENTITY_MAX> _registry;
};
