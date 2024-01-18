#pragma once

#include "ECS_System.h"
#include "GPIResource.h"

class RenderSystem : public ISystem
{
public:
	DEFINE_ECS_SYSTEM( Render );

public:
	virtual void RunSystem( std::array<std::unique_ptr<struct IComponentRegistry>, NUM_COMPONENT_MAX>& componentRegistry ) override
	{
		BeginFrame( componentRegistry );
		GeometryPass( componentRegistry );
		//DirectionalLight( componentRegistry );
		//PointLight( componentRegistry );
		//LightCombine( componentRegistry );
		//PostProcess( componentRegistry );
		EndFrame( componentRegistry );
	}

private:
	void BeginFrame( std::array<std::unique_ptr<struct IComponentRegistry>, NUM_COMPONENT_MAX>& componentRegistry );
	void GeometryPass( std::array<std::unique_ptr<struct IComponentRegistry>, NUM_COMPONENT_MAX>& componentRegistry );
	void DirectionalLight( std::array<std::unique_ptr<struct IComponentRegistry>, NUM_COMPONENT_MAX>& componentRegistry );
	void PointLight( std::array<std::unique_ptr<struct IComponentRegistry>, NUM_COMPONENT_MAX>& componentRegistry );
	void LightCombine( std::array<std::unique_ptr<struct IComponentRegistry>, NUM_COMPONENT_MAX>& componentRegistry );
	void PostProcess( std::array<std::unique_ptr<struct IComponentRegistry>, NUM_COMPONENT_MAX>& componentRegistry );
	void EndFrame( std::array<std::unique_ptr<struct IComponentRegistry>, NUM_COMPONENT_MAX>& componentRegistry );

private:
	IGPIResourceRef _swapChainResource[ 3 ];
	IGPIRenderTargetViewRef _swapChainRTV[ 3 ];
	IGPIResourceRef _swapChainDepthResource;
	IGPIDepthStencilViewRef _swapChainDSV;

	IGPIResourceRef _viewCBResource;
	IGPIConstantBufferViewRef _viewCBV;
};
