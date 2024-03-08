#pragma once

#include "ECS_System.h"
#include <RHI/RHIResource.h>

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
	IRHIResourceRef _swapChainResource[ 3 ];
	IRHIRenderTargetViewRef _swapChainRTV[ 3 ];
	IRHIResourceRef _swapChainDepthResource;
	IRHIDepthStencilViewRef _swapChainDepthDSV;

	IRHIResourceRef _viewCBResource;
	IRHIConstantBufferViewRef _viewCBV;

	IRHIResourceRef _gBufferDiffuseResource;
	IRHIResourceRef _gBufferNormalResource;
	IRHIResourceRef _gBufferUnknown0Resource;
	IRHIResourceRef _gBufferUnknown1Resource;
	IRHIRenderTargetViewRef _gBufferDiffuseRTV;
	IRHIRenderTargetViewRef _gBufferNormalRTV;
	IRHIRenderTargetViewRef _gBufferUnknown0RTV;
	IRHIRenderTargetViewRef _gBufferUnknown1RTV;
	IRHIUnorderedAccessViewRef _gBufferDiffuseUAV;
	IRHIUnorderedAccessViewRef _gBufferNormalUAV;
	IRHIUnorderedAccessViewRef _gBufferUnknown0UAV;
	IRHIUnorderedAccessViewRef _gBufferUnknown1UAV;
	IRHITextureViewTableRef _gBufferTextureViewTable;

	IRHIResourceRef _sceneLightResource;
	IRHIRenderTargetViewRef _sceneLightRTV;
	IRHIUnorderedAccessViewRef _sceneLightUAV;
	IRHIUnorderedAccessViewRef _sceneLightUAVSH;
	IRHITextureViewTableRef _sceneLightTextureViewTable;
};
