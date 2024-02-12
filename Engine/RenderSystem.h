#pragma once

#include "ECS_System.h"
#include <GPI/GPIResource.h>

class RenderSystem : public ISystem
{
public:
	DEFINE_ECS_SYSTEM( Render );

public:
	virtual void RunSystem( std::array<std::unique_ptr<struct IComponentRegistry>, NUM_COMPONENT_MAX>& componentRegistry ) override
	{
		BeginFrame( componentRegistry );
		GeometryPass( componentRegistry );
		DirectionalLight( componentRegistry );
		PointLight( componentRegistry );
		RayTracingTest( componentRegistry );
		LightCombine( componentRegistry );
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
	void RayTracingTest( std::array<std::unique_ptr<struct IComponentRegistry>, NUM_COMPONENT_MAX>& componentRegistry );
	void EndFrame( std::array<std::unique_ptr<struct IComponentRegistry>, NUM_COMPONENT_MAX>& componentRegistry );

private:
	IGPIResourceRef _swapChainResource[ 3 ];
	IGPIRenderTargetViewRef _swapChainRTV[ 3 ];
	IGPIResourceRef _swapChainDepthResource;
	IGPIDepthStencilViewRef _swapChainDepthDSV;

	IGPIResourceRef _viewCBResource;
	IGPIConstantBufferViewRef _viewCBV;

	IGPIResourceRef _gBufferDiffuseResource;
	IGPIResourceRef _gBufferNormalResource;
	IGPIResourceRef _gBufferUnknown0Resource;
	IGPIResourceRef _gBufferUnknown1Resource;
	IGPIRenderTargetViewRef _gBufferDiffuseRTV;
	IGPIRenderTargetViewRef _gBufferNormalRTV;
	IGPIRenderTargetViewRef _gBufferUnknown0RTV;
	IGPIRenderTargetViewRef _gBufferUnknown1RTV;
	IGPIUnorderedAccessViewRef _gBufferDiffuseUAV;
	IGPIUnorderedAccessViewRef _gBufferNormalUAV;
	IGPIUnorderedAccessViewRef _gBufferUnknown0UAV;
	IGPIUnorderedAccessViewRef _gBufferUnknown1UAV;
	IGPITextureViewTableRef _gBufferTextureViewTable;

	IGPIResourceRef _sceneLightResource;
	IGPIRenderTargetViewRef _sceneLightRTV;
	IGPIUnorderedAccessViewRef _sceneLightUAV;
	IGPIUnorderedAccessViewRef _sceneLightUAVSH;
	IGPITextureViewTableRef _sceneLightTextureViewTable;
};
