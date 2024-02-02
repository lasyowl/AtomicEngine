#include "RenderSystem.h"
#include "AtomicEngine.h"
#include "AssetLoader.h"
#include "SceneViewSystem.h"
#include "TransformComponent.h"
#include "PrimitiveComponent.h"
#include "KeyInputSystem.h"
#include <GPI/GPI.h>
#include <GPI/GPIPipeline.h>
#include <GPI/GPIUtility.h>
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

	uint32 swapChainIndex = AtomicEngine::GetGPI()->GetSwapChainCurrentIndex();
	IGPIResourceRef& swapChainResource = _swapChainResource[ swapChainIndex ];
	IGPIRenderTargetViewRef& swapChainRTV = _swapChainRTV[ swapChainIndex ];
	if( !swapChainResource )
	{
		swapChainResource = AtomicEngine::GetGPI()->GetSwapChainResource( swapChainIndex );

		GPIRenderTargetViewDesc rtvDesc{};
		rtvDesc.format = EGPIResourceFormat::B8G8R8A8_SRGB;
		rtvDesc.dimension = EGPIResourceDimension::Texture2D;

		swapChainRTV = AtomicEngine::GetGPI()->CreateRenderTargetView( *swapChainResource, rtvDesc );
	}

	const IVec2 windowSize = AtomicEngine::GetGPI()->GetWindowSize();

	if( !_swapChainDepthResource )
	{
		{
			const GPIResourceDesc dsDesc = GPIUtil::GetDepthStencilResourceDesc( L"DepthStencil", windowSize );

			_swapChainDepthResource = AtomicEngine::GetGPI()->CreateResource( dsDesc );

			GPIDepthStencilViewDesc dsvDesc{};
			dsvDesc.format = EGPIResourceFormat::D32_Float;
			dsvDesc.dimension = EGPIResourceDimension::Texture2D;
			dsvDesc.flag = GPIDepthStencilViewFlag_None;

			_swapChainDepthDSV = AtomicEngine::GetGPI()->CreateDepthStencilView( *_swapChainDepthResource, dsvDesc );
		}

		{
			const GPIResourceDesc cbDesc = GPIUtil::GetConstantBufferResourceDesc( L"ViewConstant", sizeof( SceneViewConstantBuffer ) );
			_viewCBResource = AtomicEngine::GetGPI()->CreateResource( cbDesc );

			GPIConstantBufferViewDesc cbvDesc{};
			cbvDesc.sizeInBytes = sizeof( SceneViewConstantBuffer );

			_viewCBV = AtomicEngine::GetGPI()->CreateConstantBufferView( *_viewCBResource, cbvDesc );
		}
	}

	if( !_gBufferDiffuseResource )
	{
		_gBufferDiffuseResource = AtomicEngine::GetGPI()->CreateResource( GPIUtil::GetRenderTargetResourceDesc( L"GBufferDiffuse", windowSize ) );
		_gBufferNormalResource = AtomicEngine::GetGPI()->CreateResource( GPIUtil::GetRenderTargetResourceDesc( L"GBufferNormal", windowSize ) );
		_gBufferUnknown0Resource = AtomicEngine::GetGPI()->CreateResource( GPIUtil::GetRenderTargetResourceDesc( L"GBufferUnknown0", windowSize ) );
		_gBufferUnknown1Resource = AtomicEngine::GetGPI()->CreateResource( GPIUtil::GetRenderTargetResourceDesc( L"GBufferUnknown1", windowSize ) );

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