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
#include <Core/Math.h>
#include <Core/Matrix.h>

void RenderSystem::GeometryPass( std::array<std::unique_ptr<IComponentRegistry>, NUM_COMPONENT_MAX>& componentRegistry )
{
	ComponentRegistry<TransformComponent>* transformCompReg = GetRegistry<TransformComponent>( componentRegistry );
	ComponentRegistry<PrimitiveComponent>* renderCompReg = GetRegistry<PrimitiveComponent>( componentRegistry );
	ComponentRegistry<KeyInputComponent>* keyInputCompReg = GetRegistry<KeyInputComponent>( componentRegistry );
	ComponentRegistry<SceneViewComponent>* sceneViewCompReg = GetRegistry<SceneViewComponent>( componentRegistry );

	KeyInputComponent& keyInputComp = keyInputCompReg->GetComponent( 0 );
	static bool bMove = true;
	if( keyInputComp.keyDown[ KeyType_P ] )
	{
		bMove = !bMove;
	}

	SceneViewComponent& sceneViewComp = sceneViewCompReg->GetComponent( 0 );

	if( !transformCompReg || !renderCompReg )
	{
		return;
	}

	static RHIPipelineStateDesc pipelineDesc{};
	static IRHIPipelineRef pipeline = nullptr;
	if( !pipeline )
	{
		pipelineDesc.id = 0;
		pipelineDesc.pipelineType = PipelineType_Graphics;
		pipelineDesc.bBindDepth = true;

		pipelineDesc.rtvFormats = {
			ERHIResourceFormat::B8G8R8A8_SRGB,
			ERHIResourceFormat::B8G8R8A8_SRGB,
			ERHIResourceFormat::B8G8R8A8_SRGB,
			ERHIResourceFormat::B8G8R8A8_SRGB
		};

		pipelineDesc.inputDesc.resize( 3 );
		pipelineDesc.inputDesc[ 0 ] = { "POSITION", ERHIResourceFormat::R32G32B32_Float, RHIInputClass_PerVertex, 0 };
		pipelineDesc.inputDesc[ 1 ] = { "NORMAL", ERHIResourceFormat::R32G32B32_Float, RHIInputClass_PerVertex, 1 };
		pipelineDesc.inputDesc[ 2 ] = { "TEXCOORD", ERHIResourceFormat::R32G32_Float, RHIInputClass_PerVertex, 2 };

		pipelineDesc.vertexShader.hash = 0;
		pipelineDesc.vertexShader.type = ShaderType_VertexShader;
		pipelineDesc.vertexShader.file = "../Engine/Shader/GeometryPass.hlsl";
		pipelineDesc.vertexShader.entry = "VS_main";
		pipelineDesc.vertexShader.macros.resize( 1 );
		pipelineDesc.vertexShader.macros[ 0 ] = { "D3D12_SAMPLE_CONSTANT_BUFFER", "1" };

		pipelineDesc.pixelShader.hash = 1;
		pipelineDesc.pixelShader.type = ShaderType_PixelShader;
		pipelineDesc.pixelShader.file = "../Engine/Shader/GeometryPass.hlsl";
		pipelineDesc.pixelShader.entry = "PS_main";
		pipelineDesc.pixelShader.macros.resize( 1 );
		pipelineDesc.pixelShader.macros[ 0 ] = { "D3D12_SAMPLE_CONSTANT_BUFFER", "1" };
		
		pipelineDesc.numCBVs = 2;

		pipeline = AtomicEngine::GetRHI()->CreatePipelineState( pipelineDesc );
	}

	{// @TODO: move to somewhere makes sense
		AtomicEngine::GetRHI()->BindRenderTargetView( *pipeline, *_gBufferDiffuseRTV, 0 );
		AtomicEngine::GetRHI()->BindRenderTargetView( *pipeline, *_gBufferNormalRTV, 1 );
		AtomicEngine::GetRHI()->BindRenderTargetView( *pipeline, *_gBufferUnknown0RTV, 2 );
		AtomicEngine::GetRHI()->BindRenderTargetView( *pipeline, *_gBufferUnknown1RTV, 3 );
		AtomicEngine::GetRHI()->BindDepthStencilView( *pipeline, *_swapChainDepthDSV );
	}

	__declspec( align( 256 ) )
		struct PrimitiveConstantBuffer
	{
		Mat4x4 matModel;
		Mat4x4 matRotation;
	};

	static IRHIResourceRef modelCBResource = nullptr;
	static IRHIConstantBufferViewRef modelCBV;
	if( !modelCBResource )
	{
		const RHIResourceDesc cbDesc = RHIUtil::GetConstantBufferResourceDesc( L"ModelConstant", sizeof( PrimitiveConstantBuffer ) );
		modelCBResource = AtomicEngine::GetRHI()->CreateResource( cbDesc );

		RHIConstantBufferViewDesc cbvDesc{};
		cbvDesc.bStatic = true;
		cbvDesc.sizeInBytes = sizeof( PrimitiveConstantBuffer );

		modelCBV = AtomicEngine::GetRHI()->CreateConstantBufferView( *modelCBResource, cbvDesc );
	}

	{// @TODO: move to somewhere makes sense
		AtomicEngine::GetRHI()->BindConstantBufferView( *pipeline, *_viewCBV, 0 );
		AtomicEngine::GetRHI()->BindConstantBufferView( *pipeline, *modelCBV, 1 );
	}

	for( Entity entity = 0; entity < NUM_ENTITY_MAX; ++entity )
	{
		if( !renderCompReg->HasComponent( entity ) )
		{
			continue;
		}

		PrimitiveComponent& primitiveComp = renderCompReg->GetComponent( entity );
		TransformComponent& transformComp = transformCompReg->GetComponent( entity );

		if( !primitiveComp.staticMeshGroup ) continue;

		Vec3 earlyTransform = Vec3( 0, 0, 0 );

		PrimitiveConstantBuffer constBuffer;
		constBuffer.matModel = AEMath::GetTransposedMatrix( AEMath::GetScaleMatrix( transformComp.scale ) * AEMath::GetTranslateMatrix( earlyTransform ) * AEMath::GetRotationMatrix( transformComp.rotation ) * AEMath::GetTranslateMatrix( transformComp.position ) );
		constBuffer.matRotation = AEMath::GetTransposedMatrix( AEMath::GetRotationMatrix( transformComp.rotation ) );

		AtomicEngine::GetRHI()->UpdateResourceData( *modelCBResource, &constBuffer, sizeof( constBuffer ) );

		AtomicEngine::GetRHI()->SetPipelineState( pipelineDesc );

		for( const StaticMesh& mesh : primitiveComp.staticMeshGroup->meshes )
		{
			AtomicEngine::GetRHI()->Render( mesh.pipelineInput );
		}

		AtomicEngine::GetRHI()->ExecuteCommandList();
	}
}