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
			dsDesc.usage = GPIResourceUsage_DepthStencil;
			dsDesc.clearValue.type = EGPIResourceClearValueType::DepthStencil;
			dsDesc.clearValue.depthStencil = Vec2( 1.0f, 0.0f );

			_swapChainDepthResource = AtomicEngine::GetGPI()->CreateResource( dsDesc );

			GPIDepthStencilViewDesc dsvDesc{};
			dsvDesc.format = EGPIResourceFormat::D32_Float;
			dsvDesc.dimension = EGPIResourceDimension::Texture2D;
			dsvDesc.flag = GPIDepthStencilViewFlag_ReadonlyDepth;

			_swapChainDSV = AtomicEngine::GetGPI()->CreateDepthStencilView( *_swapChainDepthResource, dsvDesc );
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

	SceneViewConstantBuffer viewBuffer{};
	viewBuffer.matViewProjection = AEMath::GetTransposedMatrix( sceneViewComp.matView * sceneViewComp.matProjection );
	viewBuffer.matViewProjectionInv = AEMath::GetTransposedMatrix( Mat4x4::Inverse( viewBuffer.matViewProjection ) );
	viewBuffer.viewPosition = sceneViewComp.position;
	
	AtomicEngine::GetGPI()->UpdateResourceData( *_viewCBResource, &viewBuffer, sizeof( viewBuffer ) );

	AtomicEngine::GetGPI()->BeginFrame( *swapChainResource, *swapChainRTV, *_swapChainDSV );
}