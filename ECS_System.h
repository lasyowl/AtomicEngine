#pragma once

#include "EngineEssential.h"

using SystemId = int32;

template<typename T> constexpr int GetSystemTypeId()
{
	static int32 systemId = 0;
	return systemId++;
}

class ISystem
{
public:
	virtual void RunSystem( std::array<std::unique_ptr<struct IComponentRegistry>, NUM_COMPONENT_MAX>& componentRegistry ) = 0;
};