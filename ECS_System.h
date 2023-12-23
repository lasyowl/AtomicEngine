#pragma once

#include "EngineEssential.h"

using SystemId = int32;

int32 systemTypeId = 0;
template<typename T> const int32 system_type_id = systemTypeId++;

class ISystem
{
public:
	virtual void RunSystem( std::array<std::unique_ptr<struct IComponentRegistry>, NUM_COMPONENT_MAX>& componentRegistry ) = 0;
};

void ECSInitSystems()
{
	system_type_id<class CTestSystem>;
}