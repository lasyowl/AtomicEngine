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

	static GPIPipelineStateDesc pipelineDesc{};
	static IGPIPipelineRef pipeline = nullptr;
	if( !pipeline )
	{
		pipelineDesc.id = 0;
		pipelineDesc.pipelineType = PipelineType_Graphics;
		pipelineDesc.bBindDepth = true;

		pipelineDesc.rtvFormats = {
			EGPIResourceFormat::B8G8R8A8_SRGB,
			EGPIResourceFormat::B8G8R8A8_SRGB,
			EGPIResourceFormat::B8G8R8A8_SRGB,
			EGPIResourceFormat::B8G8R8A8_SRGB
		};

		pipelineDesc.inputDesc.resize( 3 );
		pipelineDesc.inputDesc[ 0 ] = { "POSITION", EGPIResourceFormat::R32G32B32_Float, GPIInputClass_PerVertex, 0 };
		pipelineDesc.inputDesc[ 1 ] = { "NORMAL", EGPIResourceFormat::R32G32B32_Float, GPIInputClass_PerVertex, 1 };
		pipelineDesc.inputDesc[ 2 ] = { "TEXCOORD", EGPIResourceFormat::R32G32_Float, GPIInputClass_PerVertex, 2 };

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

		pipeline = AtomicEngine::GetGPI()->CreatePipelineState( pipelineDesc );
	}

	{// @TODO: move to somewhere makes sense
		AtomicEngine::GetGPI()->BindRenderTargetView( *pipeline, *_gBufferDiffuseRTV, 0 );
		AtomicEngine::GetGPI()->BindRenderTargetView( *pipeline, *_gBufferNormalRTV, 1 );
		AtomicEngine::GetGPI()->BindRenderTargetView( *pipeline, *_gBufferUnknown0RTV, 2 );
		AtomicEngine::GetGPI()->BindRenderTargetView( *pipeline, *_gBufferUnknown1RTV, 3 );
		AtomicEngine::GetGPI()->BindDepthStencilView( *pipeline, *_swapChainDepthDSV );
	}

	__declspec( align( 256 ) )
		struct PrimitiveConstantBuffer
	{
		Mat4x4 matModel;
		Mat4x4 matRotation;
	};

	static IGPIResourceRef modelCBResource = nullptr;
	static IGPIConstantBufferViewRef modelCBV;
	if( !modelCBResource )
	{
		const GPIResourceDesc cbDesc = GPIUtil::GetConstantBufferResourceDesc( L"ModelConstant", sizeof( PrimitiveConstantBuffer ) );
		modelCBResource = AtomicEngine::GetGPI()->CreateResource( cbDesc );

		GPIConstantBufferViewDesc cbvDesc{};
		cbvDesc.sizeInBytes = sizeof( PrimitiveConstantBuffer );

		modelCBV = AtomicEngine::GetGPI()->CreateConstantBufferView( *modelCBResource, cbvDesc );
	}

	{// @TODO: move to somewhere makes sense
		AtomicEngine::GetGPI()->BindConstantBufferView( *pipeline, *_viewCBV, 0 );
		AtomicEngine::GetGPI()->BindConstantBufferView( *pipeline, *modelCBV, 1 );
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
		if( entity == 2 )
		{
			static float aa = 0;
			static float sign = 1;
			if( aa > 10 || aa < -10 )
			{
				sign = -sign;
			}
			if( bMove )	aa += sign * 0.04f;
			transformComp.position = Vec3( aa, 0, -3 );
			transformComp.rotation = Vec3( 0, 0, 0 );
			transformComp.scale = Vec3( 0.5f, 0.5f, 0.5f );
		}
		else
		{
			static float aa = 0;
			//aa += 0.001f;
			transformComp.position = Vec3( 0, 0, 0 );
			transformComp.rotation = Vec3( 0, aa, 0 );
			transformComp.scale = Vec3( 1, 1, 1 );
		}

		PrimitiveConstantBuffer constBuffer;
		constBuffer.matModel = AEMath::GetTransposedMatrix( AEMath::GetScaleMatrix( transformComp.scale ) * AEMath::GetTranslateMatrix( earlyTransform ) * AEMath::GetRotationMatrix( transformComp.rotation ) * AEMath::GetTranslateMatrix( transformComp.position ) );
		constBuffer.matRotation = AEMath::GetTransposedMatrix( AEMath::GetRotationMatrix( transformComp.rotation ) );

		AtomicEngine::GetGPI()->UpdateResourceData( *modelCBResource, &constBuffer, sizeof( constBuffer ) );

		AtomicEngine::GetGPI()->SetPipelineState( pipelineDesc );

		for( const StaticMesh& mesh : primitiveComp.staticMeshGroup->meshes )
		{
			AtomicEngine::GetGPI()->Render( mesh.pipelineInput );
		}

		AtomicEngine::GetGPI()->ExecuteCommandList();
	}
}