#pragma once

#include "ECSDefine.h"

///////////////////////
// Component
///////////////////////
struct IComponentRegistry
{
	virtual ~IComponentRegistry() {}

	virtual uint32 GetComponentSize() abstract;
	virtual void AddComponent( Entity entity, void* data ) abstract;
	virtual bool HasComponent( Entity entity ) abstract;
};

template <typename T>
struct ComponentRegistry : public IComponentRegistry
{
	ComponentRegistry()
	{
		memset( _registry.data(), 0, _registry.size() * sizeof( T ) );
	}

	virtual uint32 GetComponentSize() override { return sizeof( T ); }

	virtual void AddComponent( Entity entity, void* data ) override
	{
		if( data )
		{
			memcpy( &_registry[ entity ], data, sizeof( T ) );
		}

		_signature[ entity ] = true;
	}

	virtual bool HasComponent( Entity entity ) override
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
