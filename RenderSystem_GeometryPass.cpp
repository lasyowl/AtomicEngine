#include "stdafx.h"
#include "RenderSystem.h"
#include "AtomicEngine.h"
#include "AssetLoader.h"
#include "PrimitiveComponent.h"
#include "GPI.h"
#include "GPIPipeline.h"
#include "Matrix.h"
#include "Math.h"

void RenderSystem::GeometryPass( std::array<std::unique_ptr<IComponentRegistry>, NUM_COMPONENT_MAX>& componentRegistry )
{
	ComponentRegistry<PrimitiveComponent>* renderCompReg = GetRegistry<PrimitiveComponent>( componentRegistry );
	if( !renderCompReg )
	{
		return;
	}

	__declspec( align( 256 ) )
		struct PrimitiveConstantBuffer
	{
		Mat4x4 matModel;
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
		
		pipelineDesc.constBufferSize = sizeof( PrimitiveConstantBuffer );

		AtomicEngine::GetGPI()->CreatePipelineState( pipelineDesc );
	}
	AtomicEngine::GetGPI()->SetPipelineState( pipelineDesc );

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
		static float aa = 0;
		aa += 0.001f;
		component.scale = Vec3( 1, 1, 1 );
		component.rotation = Vec3( 0, aa, 0 );
		component.translate = Vec3( 0, 0, 0 );

		PrimitiveConstantBuffer constBuffer;
		constBuffer.matModel = AEMath::GetScaleMatrix( component.scale ) * AEMath::GetRotationMatrix( component.rotation ) * AEMath::GetTranslateMatrix( component.translate );
		AtomicEngine::GetGPI()->UpdateConstantBuffer1( pipelineDesc, &constBuffer );

		for( uint32 index = 0; index < component.staticMesh->GetNumMeshes(); ++index )
		{
			AtomicEngine::GetGPI()->Render( component.positionBuffer.get(), component.uvBuffer.get(), component.normalBuffer.get(), component.indexBuffer[ index ].get() );
		}
	}

	AtomicEngine::GetGPI()->FlushPipelineState();
}