#pragma once

#include "ECS_System.h"

class RenderSystem : public ISystem
{
public:
	virtual void RunSystem( std::array<std::unique_ptr<struct IComponentRegistry>, NUM_COMPONENT_MAX>& componentRegistry ) override
	{
		RenderStaticMesh( componentRegistry );
		PostProcess( componentRegistry );
	}

private:
	void RenderStaticMesh( std::array<std::unique_ptr<struct IComponentRegistry>, NUM_COMPONENT_MAX>& componentRegistry );
	void PostProcess( std::array<std::unique_ptr<struct IComponentRegistry>, NUM_COMPONENT_MAX>& componentRegistry );
};
