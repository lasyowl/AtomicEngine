#include "stdafx.h"
#include "RenderSystem.h"
#include "AtomicEngine.h"
#include "AssetLoader.h"
#include "TransformComponent.h"
#include "PrimitiveComponent.h"
#include "KeyInputSystem.h"
#include "GPI.h"
#include "GPIPipeline.h"
#include "Matrix.h"
#include "Math.h"

void RenderSystem::GeometryPass( std::array<std::unique_ptr<IComponentRegistry>, NUM_COMPONENT_MAX>& componentRegistry )
{
	ComponentRegistry<TransformComponent>* transformCompReg = GetRegistry<TransformComponent>( componentRegistry );
	ComponentRegistry<PrimitiveComponent>* renderCompReg = GetRegistry<PrimitiveComponent>( componentRegistry );
	ComponentRegistry<KeyInputComponent>* keyInputCompReg = GetRegistry<KeyInputComponent>( componentRegistry );

	KeyInputComponent& keyInputComp = keyInputCompReg->GetComponent( 0 );
	static bool bMove = true;
	if( keyInputComp.keyDown[ KeyType_P ] )
	{
		bMove = !bMove;
	}

	if( !transformCompReg || !renderCompReg )
	{
		return;
	}

	__declspec( align( 256 ) )
		struct PrimitiveConstantBuffer
	{
		Mat4x4 matModel;
		Mat4x4 matRotation;
	};

	static GPIPipelineStateDesc pipelineDesc{};
	if( pipelineDesc.hash == 0 )
	{
		pipelineDesc.hash = 1;
		pipelineDesc.pipelineType = PipelineType_Graphics;
		pipelineDesc.bRenderSwapChainBuffer = false;
		pipelineDesc.bWriteDepth = true;
		pipelineDesc.numRenderTargets = 4;
		pipelineDesc.renderTargetDesc.resize( pipelineDesc.numRenderTargets );
		pipelineDesc.renderTargetDesc[ 0 ].format = GPIBufferFormat_B8G8R8A8_SRGB;
		pipelineDesc.renderTargetDesc[ 1 ].format = GPIBufferFormat_B8G8R8A8_SRGB;
		pipelineDesc.renderTargetDesc[ 2 ].format = GPIBufferFormat_B8G8R8A8_SRGB;
		pipelineDesc.renderTargetDesc[ 3 ].format = GPIBufferFormat_B8G8R8A8_SRGB;

		pipelineDesc.inputDesc.resize( 3 );
		pipelineDesc.inputDesc[ 0 ] = { "POSITION", GPIBufferFormat_R32G32B32_Float, GPIInputClass_PerVertex, 0 };
		pipelineDesc.inputDesc[ 1 ] = { "NORMAL", GPIBufferFormat_R32G32B32_Float, GPIInputClass_PerVertex, 1 };
		pipelineDesc.inputDesc[ 2 ] = { "TEXCOORD", GPIBufferFormat_R32G32_Float, GPIInputClass_PerVertex, 2 };

		pipelineDesc.vertexShader.hash = 1;
		pipelineDesc.vertexShader.type = ShaderType_VertexShader;
		pipelineDesc.vertexShader.file = "Engine/Shader/GeometryPass.hlsl";
		pipelineDesc.vertexShader.entry = "VS_main";
		pipelineDesc.vertexShader.macros.resize( 1 );
		pipelineDesc.vertexShader.macros[ 0 ] = { "D3D12_SAMPLE_CONSTANT_BUFFER", "1" };

		pipelineDesc.pixelShader.hash = 2;
		pipelineDesc.pixelShader.type = ShaderType_PixelShader;
		pipelineDesc.pixelShader.file = "Engine/Shader/GeometryPass.hlsl";
		pipelineDesc.pixelShader.entry = "PS_main";
		pipelineDesc.pixelShader.macros.resize( 1 );
		pipelineDesc.pixelShader.macros[ 0 ] = { "D3D12_SAMPLE_CONSTANT_BUFFER", "1" };
		
		pipelineDesc.numConstantBuffers = 1;
		pipelineDesc.constBufferSize = sizeof( PrimitiveConstantBuffer );

		pipelineDesc.numResources = 0;

		AtomicEngine::GetGPI()->CreatePipelineState( pipelineDesc );
	}

	for( Entity entity = 0; entity < NUM_ENTITY_MAX; ++entity )
	{
		if( !renderCompReg->HasComponent( entity ) )
		{
			continue;
		}

		PrimitiveComponent& component = renderCompReg->GetComponent( entity );
		if( !component.positionBuffer )
		{
			std::shared_ptr<StaticMesh>& staticMesh = component.staticMesh;

			component.positionBuffer = AtomicEngine::GetGPI()->CreateVertexBuffer( staticMesh->GetPositionPtr(), staticMesh->GetPositionStride(), staticMesh->GetPositionByteSize() );
			if( !staticMesh->normal.empty() )
			{
				component.normalBuffer = AtomicEngine::GetGPI()->CreateVertexBuffer( staticMesh->GetNormalPtr(), staticMesh->GetNormalStride(), staticMesh->GetNormalByteSize() );
			}
			else
			{
				component.normalBuffer = nullptr;
			}
			if( !staticMesh->uv.empty() )
			{
				component.uvBuffer = AtomicEngine::GetGPI()->CreateVertexBuffer( staticMesh->GetUVPtr(), staticMesh->GetUVStride(), staticMesh->GetUVByteSize() );
			}
			else
			{
				component.uvBuffer = nullptr;
			}

			for( uint32 index = 0; index < staticMesh->GetNumMeshes(); ++index )
			{
				IIndexBufferRef indexBuffer = AtomicEngine::GetGPI()->CreateIndexBuffer( staticMesh->GetIndexPtr( index ), staticMesh->GetIndexByteSize( index ) );
				component.indexBuffer.emplace_back( indexBuffer );
			}
		}

		TransformComponent& transformComp = transformCompReg->GetComponent( entity );

		Vec3 earlyTransform = Vec3( 0, 0, 0 );
		if( entity == 3 )
		{
			static float aa = 0;
			static float sign = 1;
			if( aa > 10 || aa < -10 )
			{
				sign = -sign;
			}
			if( bMove )	aa += sign * 0.04f;
			transformComp.position = Vec3( aa, 0, -5 );
			transformComp.rotation = Vec3( 0, 0, 0 );
			transformComp.scale = Vec3( 0.5f, 0.5f, 0.5f );
		}
		else
		{
			static float aa = 0;
			aa += 0.001f;
			transformComp.position = Vec3( 0, 0, 0 );
			transformComp.rotation = Vec3( 0, aa, 0 );
			transformComp.scale = Vec3( 1, 1, 1 );
		}

		AtomicEngine::GetGPI()->SetPipelineState( pipelineDesc );

		PrimitiveConstantBuffer constBuffer;
		constBuffer.matModel = AEMath::GetTransposedMatrix( AEMath::GetScaleMatrix( transformComp.scale ) * AEMath::GetTranslateMatrix( earlyTransform ) * AEMath::GetRotationMatrix( transformComp.rotation ) * AEMath::GetTranslateMatrix( transformComp.position ) );
		constBuffer.matRotation = AEMath::GetTransposedMatrix( AEMath::GetRotationMatrix( transformComp.rotation ) );
		AtomicEngine::GetGPI()->UpdateConstantBuffer1( pipelineDesc, &constBuffer );

		for( uint32 index = 0; index < component.staticMesh->GetNumMeshes(); ++index )
		{
			AtomicEngine::GetGPI()->Render( component.positionBuffer.get(), component.uvBuffer.get(), component.normalBuffer.get(), component.indexBuffer[ index ].get() );
		}

		AtomicEngine::GetGPI()->FlushPipelineState();
	}
}