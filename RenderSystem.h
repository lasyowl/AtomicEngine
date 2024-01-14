#pragma once

#include "ECS_System.h"
#include "Matrix.h"

class RenderSystem : public ISystem
{
public:
	DEFINE_ECS_SYSTEM( Render );
public:
	virtual void RunSystem( std::array<std::unique_ptr<struct IComponentRegistry>, NUM_COMPONENT_MAX>& componentRegistry ) override
	{
		GeometryPass( componentRegistry );
		DirectionalLight( componentRegistry );
		PointLight( componentRegistry );
		LightCombine( componentRegistry );
		PostProcess( componentRegistry );
	}

private:
	void GeometryPass( std::array<std::unique_ptr<struct IComponentRegistry>, NUM_COMPONENT_MAX>& componentRegistry );
	void DirectionalLight( std::array<std::unique_ptr<struct IComponentRegistry>, NUM_COMPONENT_MAX>& componentRegistry );
	void PointLight( std::array<std::unique_ptr<struct IComponentRegistry>, NUM_COMPONENT_MAX>& componentRegistry );
	void LightCombine( std::array<std::unique_ptr<struct IComponentRegistry>, NUM_COMPONENT_MAX>& componentRegistry );
	void PostProcess( std::array<std::unique_ptr<struct IComponentRegistry>, NUM_COMPONENT_MAX>& componentRegistry );
};
