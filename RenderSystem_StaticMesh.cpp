#include "stdafx.h"
#include "RenderSystem.h"
#include "AtomicEngine.h"
#include "AssetLoader.h"
#include "PrimitiveComponent.h"
#include "GPI.h"
#include "GPIPipeline.h"

void RenderSystem::RenderStaticMesh( std::array<std::unique_ptr<struct IComponentRegistry>, NUM_COMPONENT_MAX>& componentRegistry )
{
	ComponentRegistry<PrimitiveComponent>* renderCompReg = GetRegistry<PrimitiveComponent>( componentRegistry );
	if( !renderCompReg )
	{
		return;
	}

	static GPIPipelineStateDesc pipelineDesc{};
	if( pipelineDesc.hash == 0 )
	{
		pipelineDesc.hash = 1;
		pipelineDesc.bRenderSwapChainBuffer = false;
		pipelineDesc.numRenderTargets = 4;
		pipelineDesc.pipelineType = PipelineType_Graphics;

		pipelineDesc.inputDesc.resize( 3 );
		pipelineDesc.inputDesc[ 0 ] = { "POSITION", GPIDataFormat_R32G32B32_Float, GPIInputClass_PerVertex, 0 };
		pipelineDesc.inputDesc[ 1 ] = { "NORMAL", GPIDataFormat_R32G32B32_Float, GPIInputClass_PerVertex, 1 };
		pipelineDesc.inputDesc[ 2 ] = { "TEXCOORD", GPIDataFormat_R32G32_Float, GPIInputClass_PerVertex, 2 };

		pipelineDesc.vertexShader.hash = 1;
		pipelineDesc.vertexShader.type = ShaderType_VertexShader;
		pipelineDesc.vertexShader.file = "Engine/Shader/Test.hlsl";
		pipelineDesc.vertexShader.entry = "VS_main";
		pipelineDesc.vertexShader.macros.resize( 1 );
		pipelineDesc.vertexShader.macros[ 0 ] = { "D3D12_SAMPLE_CONSTANT_BUFFER", "1" };

		pipelineDesc.pixelShader.hash = 2;
		pipelineDesc.pixelShader.type = ShaderType_PixelShader;
		pipelineDesc.pixelShader.file = "Engine/Shader/Test.hlsl";
		pipelineDesc.pixelShader.entry = "PS_main";
		pipelineDesc.pixelShader.macros.resize( 1 );
		pipelineDesc.pixelShader.macros[ 0 ] = { "D3D12_SAMPLE_CONSTANT_BUFFER", "1" };
		
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
			//component.staticMesh = AssetLoader::LoadStaticMesh( "Resource/Sponza-master/sponza.obj" );
			component.staticMesh = AssetLoader::LoadStaticMesh( "Resource/teapot.obj" );
			//component.staticMesh = AssetLoader::LoadStaticMesh( "Resource/teapot.obj" );
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
		for( uint32 index = 0; index < component.staticMesh->GetNumMeshes(); ++index )
		{
			AtomicEngine::GetGPI()->Render( component.positionBuffer.get(), component.uvBuffer.get(), component.normalBuffer.get(), component.indexBuffer[ index ].get() );
		}
	}

	AtomicEngine::GetGPI()->FlushPipelineState();
}