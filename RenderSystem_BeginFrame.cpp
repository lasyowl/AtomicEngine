#include "stdafx.h"
#include "RenderSystem.h"
#include "AtomicEngine.h"
#include "AssetLoader.h"
#include "SceneViewSystem.h"
#include "TransformComponent.h"
#include "PrimitiveComponent.h"
#include "KeyInputSystem.h"
#include "GPI.h"
#include "GPIPipeline.h"
#include "Matrix.h"
#include "Math.h"
#include "SceneViewSystem.h"

void RenderSystem::BeginFrame( std::array<std::unique_ptr<IComponentRegistry>, NUM_COMPONENT_MAX>& componentRegistry )
{
	ComponentRegistry<SceneViewComponent>* viewCompReg = GetRegistry<SceneViewComponent>( componentRegistry );
	if( !viewCompReg )
	{
		return;
	}

	SceneViewComponent& sceneViewComp = viewCompReg->GetComponent( 0 );

	uint32 swapChainIndex = AtomicEngine::GetGPI()->GetSwapChainCurrentIndex();
	IGPIResourceRef& swapChainResource = _swapChainResource[ swapChainIndex ];
	IGPIRenderTargetViewRef& swapChainRTV = _swapChainRTV[ swapChainIndex ];
	if( !swapChainResource )
	{
		{
			swapChainResource = AtomicEngine::GetGPI()->GetSwapChainResource( swapChainIndex );

			GPIRenderTargetViewDesc rtvDesc{};
			rtvDesc.format = EGPIResourceFormat::B8G8R8A8_SRGB;
			rtvDesc.dimension = EGPIResourceDimension::Texture2D;

			swapChainRTV = AtomicEngine::GetGPI()->CreateRenderTargetView( *swapChainResource, rtvDesc );
		}

		{
			GPIResourceDesc dsDesc{};
			dsDesc.dimension = EGPIResourceDimension::Texture2D;
			dsDesc.format = EGPIResourceFormat::D32_Float;
			dsDesc.width = 1920;
			dsDesc.height = 1080;
			dsDesc.depth = 1;
			dsDesc.numMips = 1;
			dsDesc.flags = GPIResourceFlag_AllowDepthStencil;
			dsDesc.initialState = GPIResourceState_DepthWrite;
			dsDesc.clearValue.type = EGPIResourceClearValueType::DepthStencil;
			dsDesc.clearValue.depthStencil = Vec2( 1.0f, 0.0f );

			_swapChainDepthResource = AtomicEngine::GetGPI()->CreateResource( dsDesc );

			GPIDepthStencilViewDesc dsvDesc{};
			dsvDesc.format = EGPIResourceFormat::D32_Float;
			dsvDesc.dimension = EGPIResourceDimension::Texture2D;
			dsvDesc.flag = GPIDepthStencilViewFlag_None;

			_swapChainDepthDSV = AtomicEngine::GetGPI()->CreateDepthStencilView( *_swapChainDepthResource, dsvDesc );

			GPIShaderResourceViewDesc srvDesc{};
			srvDesc.format = EGPIResourceFormat::R32_Float;
			srvDesc.dimension = EGPIResourceDimension::Texture2D;

			_swapChainDepthSRV = AtomicEngine::GetGPI()->CreateShaderResourceView( *_swapChainDepthResource, srvDesc );
		}

		{
			GPIResourceDesc cbDesc{};
			cbDesc.dimension = EGPIResourceDimension::Buffer;
			cbDesc.format = EGPIResourceFormat::Unknown;
			cbDesc.width = sizeof( SceneViewConstantBuffer );
			cbDesc.height = 1;
			cbDesc.depth = 1;
			cbDesc.numMips = 1;
			cbDesc.flags = GPIResourceFlag_None;

			_viewCBResource = AtomicEngine::GetGPI()->CreateResource( cbDesc );

			GPIConstantBufferViewDesc cbvDesc{};
			cbvDesc.sizeInBytes = sizeof( SceneViewConstantBuffer );

			_viewCBV = AtomicEngine::GetGPI()->CreateConstantBufferView( *_viewCBResource, cbvDesc );
		}
	}

	if( !_gBufferDiffuseResource )
	{
		GPIResourceDesc gBufferDesc{};
		gBufferDesc.dimension = EGPIResourceDimension::Texture2D;
		gBufferDesc.format = EGPIResourceFormat::B8G8R8A8;
		gBufferDesc.width = 1920;
		gBufferDesc.height = 1080;
		gBufferDesc.depth = 1;
		gBufferDesc.numMips = 1;
		gBufferDesc.flags = GPIResourceFlag_AllowRenderTarget | GPIResourceFlag_AllowUnorderedAccess;
		gBufferDesc.initialState = GPIResourceState_RenderTarget;
		gBufferDesc.clearValue.type = EGPIResourceClearValueType::Color;
		gBufferDesc.clearValue.color = Vec4( 0.0f, 0.0f, 0.0f, 0.0f );

		_gBufferDiffuseResource = AtomicEngine::GetGPI()->CreateResource( gBufferDesc );
		_gBufferNormalResource = AtomicEngine::GetGPI()->CreateResource( gBufferDesc );
		_gBufferUnknown0Resource = AtomicEngine::GetGPI()->CreateResource( gBufferDesc );
		_gBufferUnknown1Resource = AtomicEngine::GetGPI()->CreateResource( gBufferDesc );

		GPIRenderTargetViewDesc rtvDesc{};
		rtvDesc.format = EGPIResourceFormat::B8G8R8A8_SRGB;
		rtvDesc.dimension = EGPIResourceDimension::Texture2D;

		_gBufferDiffuseRTV = AtomicEngine::GetGPI()->CreateRenderTargetView( *_gBufferDiffuseResource, rtvDesc );
		_gBufferNormalRTV = AtomicEngine::GetGPI()->CreateRenderTargetView( *_gBufferNormalResource, rtvDesc );
		_gBufferUnknown0RTV = AtomicEngine::GetGPI()->CreateRenderTargetView( *_gBufferUnknown0Resource, rtvDesc );
		_gBufferUnknown1RTV = AtomicEngine::GetGPI()->CreateRenderTargetView( *_gBufferUnknown1Resource, rtvDesc );

		GPIShaderResourceViewDesc srvDesc{};
		srvDesc.format = EGPIResourceFormat::B8G8R8A8_SRGB;
		srvDesc.dimension = EGPIResourceDimension::Texture2D;

		GPIShaderResourceViewDesc textureDesc{};
		textureDesc.format = EGPIResourceFormat::R32_Float;
		textureDesc.dimension = EGPIResourceDimension::Texture2D;

		std::vector<const IGPIResource*> textureResources = {
			_gBufferDiffuseResource.get(),
			_gBufferNormalResource.get(),
			_gBufferUnknown0Resource.get(),
			_swapChainDepthResource.get()
		};
		std::vector<GPIShaderResourceViewDesc> textureDescs = {
			srvDesc,
			srvDesc,
			srvDesc,
			textureDesc
		};
		_gBufferTextureViewTable = AtomicEngine::GetGPI()->CreateTextureViewTable( textureResources, textureDescs );

		GPIUnorderedAccessViewDesc uavDesc{};
		uavDesc.format = EGPIResourceFormat::R32_Uint;
		uavDesc.dimension = EGPIResourceDimension::Texture2D;

		_gBufferDiffuseUAV = AtomicEngine::GetGPI()->CreateUnorderedAccessView( *_gBufferDiffuseResource, uavDesc, true );
		_gBufferNormalUAV = AtomicEngine::GetGPI()->CreateUnorderedAccessView( *_gBufferNormalResource, uavDesc, true );
		_gBufferUnknown0UAV = AtomicEngine::GetGPI()->CreateUnorderedAccessView( *_gBufferUnknown0Resource, uavDesc, true );
		_gBufferUnknown1UAV = AtomicEngine::GetGPI()->CreateUnorderedAccessView( *_gBufferUnknown1Resource, uavDesc, true );
	}

	SceneViewConstantBuffer viewBuffer{};
	viewBuffer.matViewProjection = AEMath::GetTransposedMatrix( sceneViewComp.matView * sceneViewComp.matProjection );
	viewBuffer.matViewProjectionInv = AEMath::GetTransposedMatrix( Mat4x4::Inverse( viewBuffer.matViewProjection ) );
	viewBuffer.viewPosition = sceneViewComp.position;
	
	AtomicEngine::GetGPI()->BeginFrame( *swapChainResource, *swapChainRTV, *_swapChainDepthDSV );

	AtomicEngine::GetGPI()->UpdateResourceData( *_viewCBResource, &viewBuffer, sizeof( viewBuffer ) );

	AtomicEngine::GetGPI()->ClearRenderTarget( *_gBufferDiffuseUAV );
	AtomicEngine::GetGPI()->ClearRenderTarget( *_gBufferNormalUAV );
	AtomicEngine::GetGPI()->ClearRenderTarget( *_gBufferUnknown0UAV );
	AtomicEngine::GetGPI()->ClearRenderTarget( *_gBufferUnknown1UAV );
}