#include "RenderSystem.h"
#include "AtomicEngine.h"
#include "AssetLoader.h"
#include "SceneViewSystem.h"
#include "TransformComponent.h"
#include "PrimitiveComponent.h"
#include "KeyInputSystem.h"
#include <RHI/RHI.h>
#include <RHI/RHIPipeline.h>
#include <RHI/RHIUtility.h>
#include <Core/Matrix.h>
#include <Core/Math.h>
#include "SceneViewSystem.h"

void RenderSystem::BeginFrame( std::array<std::unique_ptr<IComponentRegistry>, NUM_COMPONENT_MAX>& componentRegistry )
{
	ComponentRegistry<SceneViewComponent>* viewCompReg = GetRegistry<SceneViewComponent>( componentRegistry );
	if( !viewCompReg )
	{
		return;
	}

	SceneViewComponent& sceneViewComp = viewCompReg->GetComponent( 0 );

	uint32 swapChainIndex = AtomicEngine::GetRHI()->GetSwapChainCurrentIndex();
	IRHIResourceRef& swapChainResource = _swapChainResource[ swapChainIndex ];
	IRHIRenderTargetViewRef& swapChainRTV = _swapChainRTV[ swapChainIndex ];
	if( !swapChainResource )
	{
		swapChainResource = AtomicEngine::GetRHI()->GetSwapChainResource( swapChainIndex );

		RHIRenderTargetViewDesc rtvDesc{};
		rtvDesc.format = ERHIResourceFormat::B8G8R8A8_SRGB;
		rtvDesc.dimension = ERHIResourceDimension::Texture2D;

		swapChainRTV = AtomicEngine::GetRHI()->CreateRenderTargetView( *swapChainResource, rtvDesc );
	}

	const IVec2 windowSize = AtomicEngine::GetRHI()->GetWindowSize();

	if( !_swapChainDepthResource )
	{
		{
			const RHIResourceDesc dsDesc = RHIUtil::GetDepthStencilResourceDesc( L"DepthStencil", windowSize );

			_swapChainDepthResource = AtomicEngine::GetRHI()->CreateResource( dsDesc );

			RHIDepthStencilViewDesc dsvDesc{};
			dsvDesc.format = ERHIResourceFormat::D32_Float;
			dsvDesc.dimension = ERHIResourceDimension::Texture2D;
			dsvDesc.flag = RHIDepthStencilViewFlag_None;

			_swapChainDepthDSV = AtomicEngine::GetRHI()->CreateDepthStencilView( *_swapChainDepthResource, dsvDesc );
		}

		{
			const RHIResourceDesc cbDesc = RHIUtil::GetConstantBufferResourceDesc( L"ViewConstant", sizeof( SceneViewConstantBuffer ) );
			_viewCBResource = AtomicEngine::GetRHI()->CreateResource( cbDesc );

			RHIConstantBufferViewDesc cbvDesc{};
			cbvDesc.sizeInBytes = sizeof( SceneViewConstantBuffer );

			_viewCBV = AtomicEngine::GetRHI()->CreateConstantBufferView( *_viewCBResource, cbvDesc );
		}
	}

	if( !_gBufferDiffuseResource )
	{
		_gBufferDiffuseResource = AtomicEngine::GetRHI()->CreateResource( RHIUtil::GetRenderTargetResourceDesc( L"GBufferDiffuse", windowSize ) );
		_gBufferNormalResource = AtomicEngine::GetRHI()->CreateResource( RHIUtil::GetRenderTargetResourceDesc( L"GBufferNormal", windowSize ) );
		_gBufferUnknown0Resource = AtomicEngine::GetRHI()->CreateResource( RHIUtil::GetRenderTargetResourceDesc( L"GBufferUnknown0", windowSize ) );
		_gBufferUnknown1Resource = AtomicEngine::GetRHI()->CreateResource( RHIUtil::GetRenderTargetResourceDesc( L"GBufferUnknown1", windowSize ) );

		RHIRenderTargetViewDesc rtvDesc{};
		rtvDesc.format = ERHIResourceFormat::B8G8R8A8_SRGB;
		rtvDesc.dimension = ERHIResourceDimension::Texture2D;

		_gBufferDiffuseRTV = AtomicEngine::GetRHI()->CreateRenderTargetView( *_gBufferDiffuseResource, rtvDesc );
		_gBufferNormalRTV = AtomicEngine::GetRHI()->CreateRenderTargetView( *_gBufferNormalResource, rtvDesc );
		_gBufferUnknown0RTV = AtomicEngine::GetRHI()->CreateRenderTargetView( *_gBufferUnknown0Resource, rtvDesc );
		_gBufferUnknown1RTV = AtomicEngine::GetRHI()->CreateRenderTargetView( *_gBufferUnknown1Resource, rtvDesc );

		RHIShaderResourceViewDesc srvDesc{};
		srvDesc.format = ERHIResourceFormat::B8G8R8A8_SRGB;
		srvDesc.dimension = ERHIResourceDimension::Texture2D;

		RHIShaderResourceViewDesc textureDesc{};
		textureDesc.format = ERHIResourceFormat::R32_Float;
		textureDesc.dimension = ERHIResourceDimension::Texture2D;

		std::vector<const IRHIResource*> textureResources = {
			_gBufferDiffuseResource.get(),
			_gBufferNormalResource.get(),
			_gBufferUnknown0Resource.get(),
			_swapChainDepthResource.get()
		};
		std::vector<RHIShaderResourceViewDesc> textureDescs = {
			srvDesc,
			srvDesc,
			srvDesc,
			textureDesc
		};
		_gBufferTextureViewTable = AtomicEngine::GetRHI()->CreateTextureViewTable( textureResources, textureDescs );

		RHIUnorderedAccessViewDesc uavDesc{};
		uavDesc.format = ERHIResourceFormat::R32_Uint;
		uavDesc.dimension = ERHIResourceDimension::Texture2D;

		_gBufferDiffuseUAV = AtomicEngine::GetRHI()->CreateUnorderedAccessView( *_gBufferDiffuseResource, uavDesc, true );
		_gBufferNormalUAV = AtomicEngine::GetRHI()->CreateUnorderedAccessView( *_gBufferNormalResource, uavDesc, true );
		_gBufferUnknown0UAV = AtomicEngine::GetRHI()->CreateUnorderedAccessView( *_gBufferUnknown0Resource, uavDesc, true );
		_gBufferUnknown1UAV = AtomicEngine::GetRHI()->CreateUnorderedAccessView( *_gBufferUnknown1Resource, uavDesc, true );
	}

	SceneViewConstantBuffer viewBuffer{};
	viewBuffer.matViewProjection = AEMath::GetTransposedMatrix( sceneViewComp.matView * sceneViewComp.matProjection );
	viewBuffer.matViewProjectionInv = AEMath::GetTransposedMatrix( Mat4x4::Inverse( viewBuffer.matViewProjection ) );
	viewBuffer.viewPosition = sceneViewComp.position;
	
	AtomicEngine::GetRHI()->BeginFrame( *swapChainResource, *swapChainRTV, *_swapChainDepthDSV );

	AtomicEngine::GetRHI()->UpdateResourceData( *_viewCBResource, &viewBuffer, sizeof( viewBuffer ) );

	AtomicEngine::GetRHI()->ClearRenderTarget( *_gBufferDiffuseUAV );
	AtomicEngine::GetRHI()->ClearRenderTarget( *_gBufferNormalUAV );
	AtomicEngine::GetRHI()->ClearRenderTarget( *_gBufferUnknown0UAV );
	AtomicEngine::GetRHI()->ClearRenderTarget( *_gBufferUnknown1UAV );
}