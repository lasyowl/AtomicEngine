#include "stdafx.h"
#include "RenderSystem.h"
#include "AtomicEngine.h"
#include "GPI.h"
#include "GPIPipeline.h"
#include "StaticMesh.h"
#include "SampleMesh.h"
#include "SceneViewSystem.h"
#include "LightComponent.h"
#include "TransformComponent.h"
#include "PrimitiveComponent.h"
#include "Math.h"

void RenderSystem::DirectionalLight( std::array<std::unique_ptr<IComponentRegistry>, NUM_COMPONENT_MAX>& componentRegistry )
{
	ComponentRegistry<SceneViewComponent>* sceneViewCompReg = GetRegistry<SceneViewComponent>( componentRegistry );

	SceneViewComponent& sceneViewComp = sceneViewCompReg->GetComponent( 0 );

	static GPIPipelineStateDesc pipelineDesc{};
	if( pipelineDesc.id == 0 )
	{
		pipelineDesc.id = 2;
		pipelineDesc.pipelineType = PipelineType_Graphics;
		pipelineDesc.bRenderSwapChainBuffer = true;
		pipelineDesc.bWriteDepth = false;

		pipelineDesc.rtvFormats = { EGPIResourceFormat::B8G8R8A8_SRGB };

		pipelineDesc.inputDesc.resize( 2 );
		pipelineDesc.inputDesc[ 0 ] = { "POSITION", EGPIResourceFormat::R32G32B32_Float, GPIInputClass_PerVertex, 0 };
		pipelineDesc.inputDesc[ 1 ] = { "TEXCOORD", EGPIResourceFormat::R32G32_Float, GPIInputClass_PerVertex, 2 };

		pipelineDesc.vertexShader.hash = 3;
		pipelineDesc.vertexShader.type = ShaderType_VertexShader;
		pipelineDesc.vertexShader.file = "Engine/Shader/Lighting.hlsl";
		pipelineDesc.vertexShader.entry = "VS_DirectionalLight";

		pipelineDesc.pixelShader.hash = 4;
		pipelineDesc.pixelShader.type = ShaderType_PixelShader;
		pipelineDesc.pixelShader.file = "Engine/Shader/Lighting.hlsl";
		pipelineDesc.pixelShader.entry = "PS_DirectionalLight";

		pipelineDesc.numCBVs = 1;

		AtomicEngine::GetGPI()->CreatePipelineState( pipelineDesc );

		/*struct PointLightResourceBuffer
		{
			std::vector<Vec3> position;
			std::vector<float> radius;

			IVertexBufferRef positionBuffer;
			IVertexBufferRef radiusBuffer;
		} static pointLightBuffer;
		for( int32 iter = 0; iter < 100; ++iter )
		{
			pointLightBuffer.position.emplace_back( Vec3(1 + iter, 2 + iter, 3 + iter ) );
			pointLightBuffer.radius.emplace_back( 1.0f );
		}
		pointLightBuffer.positionBuffer = AtomicEngine::GetGPI()->CreateResourceBuffer( pointLightBuffer.position.data(), pointLightBuffer.position.size() * sizeof( Vec3 ) );

		AtomicEngine::GetGPI()->BindResourceBuffer( pipelineDesc, pointLightBuffer.positionBuffer.get(), 0);*/
	}
	//AtomicEngine::GetGPI()->BindConstantBuffer( pipelineDesc, sceneViewComp.constBufferID, 0 );

	/*AtomicEngine::GetGPI()->SetPipelineState( pipelineDesc );

	static IVertexBufferRef positionBuffer = nullptr;
	static IVertexBufferRef uvBuffer = nullptr;
	static IIndexBufferRef indexBuffer = nullptr;

	if( !positionBuffer )
	{
		static StaticMesh staticMesh = SampleMesh::GetQuad();

		positionBuffer = AtomicEngine::GetGPI()->CreateVertexBuffer( staticMesh.GetPositionPtr(), staticMesh.GetPositionStride(), staticMesh.GetPositionByteSize() );
		uvBuffer = AtomicEngine::GetGPI()->CreateVertexBuffer( staticMesh.GetUVPtr(), staticMesh.GetUVStride(), staticMesh.GetUVByteSize() );
		indexBuffer = AtomicEngine::GetGPI()->CreateIndexBuffer( staticMesh.GetIndexPtr( 0 ), staticMesh.GetIndexByteSize( 0 ) );
	}

	AtomicEngine::GetGPI()->Render( positionBuffer.get(), uvBuffer.get(), nullptr, indexBuffer.get());

	AtomicEngine::GetGPI()->ExecuteCommandList();*/
}

void RenderSystem::PointLight( std::array<std::unique_ptr<IComponentRegistry>, NUM_COMPONENT_MAX>& componentRegistry )
{
	ComponentRegistry<TransformComponent>* transformCompReg = GetRegistry<TransformComponent>( componentRegistry );
	ComponentRegistry<PrimitiveComponent>* renderCompReg = GetRegistry<PrimitiveComponent>( componentRegistry );
	ComponentRegistry<LightComponent>* lightCompReg = GetRegistry<LightComponent>( componentRegistry );
	ComponentRegistry<SceneViewComponent>* sceneViewCompReg = GetRegistry<SceneViewComponent>( componentRegistry );

	if( !transformCompReg || !renderCompReg || !lightCompReg )
	{
		return;
	}

	SceneViewComponent& sceneViewComp = sceneViewCompReg->GetComponent( 0 );

	__declspec( align( 256 ) )
	struct LightConstantBuffer
	{
		Mat4x4 matModel;
		Vec3 origin;
		float intensity;
	};

	static GPIPipelineStateDesc pipelineDesc{};
	if( pipelineDesc.id == 0 )
	{
		pipelineDesc.id = 3;
		pipelineDesc.pipelineType = PipelineType_Graphics;
		pipelineDesc.bRenderSwapChainBuffer = true;
		pipelineDesc.bWriteDepth = false;

		pipelineDesc.rtvFormats = { EGPIResourceFormat::B8G8R8A8_SRGB };

		pipelineDesc.inputDesc.resize( 2 );
		pipelineDesc.inputDesc[ 0 ] = { "POSITION", EGPIResourceFormat::R32G32B32_Float, GPIInputClass_PerVertex, 0 };
		pipelineDesc.inputDesc[ 1 ] = { "TEXCOORD", EGPIResourceFormat::R32G32_Float, GPIInputClass_PerVertex, 2 };

		pipelineDesc.vertexShader.hash = 5;
		pipelineDesc.vertexShader.type = ShaderType_VertexShader;
		pipelineDesc.vertexShader.file = "Engine/Shader/Lighting.hlsl";
		pipelineDesc.vertexShader.entry = "VS_PointLight";

		pipelineDesc.pixelShader.hash = 6;
		pipelineDesc.pixelShader.type = ShaderType_PixelShader;
		pipelineDesc.pixelShader.file = "Engine/Shader/Lighting.hlsl";
		pipelineDesc.pixelShader.entry = "PS_PointLight";

		pipelineDesc.numCBVs = 2;
		pipelineDesc.numSRVs = 0;

		AtomicEngine::GetGPI()->CreatePipelineState( pipelineDesc );
	}

	for( Entity entity = 0; entity < NUM_ENTITY_MAX; ++entity )
	{
		if( !lightCompReg->HasComponent( entity ) )
		{
			continue;
		}

		LightComponent& lightComp = lightCompReg->GetComponent( entity );
		PrimitiveComponent& renderComp = renderCompReg->GetComponent( entity );
		TransformComponent& transformComp = transformCompReg->GetComponent( entity );

		Vec3 earlyTransform = Vec3( 0, 0, 0 );

		LightConstantBuffer constBuffer;
		constBuffer.matModel = AEMath::GetTransposedMatrix( AEMath::GetScaleMatrix( Vec3( 3, 3, 3 )/*transformComp.scale*/ ) * AEMath::GetTranslateMatrix( earlyTransform ) * AEMath::GetRotationMatrix( transformComp.rotation ) * AEMath::GetTranslateMatrix( transformComp.position ) );
		constBuffer.origin = transformComp.position;
		constBuffer.intensity = 3.0f;

		//AtomicEngine::GetGPI()->BindConstantBuffer( pipelineDesc, sceneViewComp.constBufferID, 0 );

		//static const uint32 constBufferID = AtomicEngine::GetGPI()->CreateConstantBuffer( &constBuffer, sizeof( LightConstantBuffer ) );
		//AtomicEngine::GetGPI()->UpdateConstantBuffer( constBufferID, &constBuffer, sizeof( LightConstantBuffer ) );
		//AtomicEngine::GetGPI()->BindConstantBuffer( pipelineDesc, constBufferID, 1 );

		AtomicEngine::GetGPI()->SetPipelineState( pipelineDesc );

		for( uint32 index = 0; index < renderComp.staticMesh->GetNumMeshes(); ++index )
		{
			//AtomicEngine::GetGPI()->Render( renderComp.positionBuffer.get(), renderComp.uvBuffer.get(), nullptr, renderComp.indexBuffer[ index ].get() );
		}

		AtomicEngine::GetGPI()->ExecuteCommandList();
	}
}

void RenderSystem::LightCombine( std::array<std::unique_ptr<IComponentRegistry>, NUM_COMPONENT_MAX>& componentRegistry )
{
	ComponentRegistry<SceneViewComponent>* sceneViewCompReg = GetRegistry<SceneViewComponent>( componentRegistry );

	SceneViewComponent& sceneViewComp = sceneViewCompReg->GetComponent( 0 );

	static GPIPipelineStateDesc pipelineDesc{};
	if( pipelineDesc.id == 0 )
	{
		pipelineDesc.id = 4;
		pipelineDesc.pipelineType = PipelineType_Graphics;
		pipelineDesc.bRenderSwapChainBuffer = true;
		pipelineDesc.bWriteDepth = false;

		pipelineDesc.rtvFormats = { EGPIResourceFormat::B8G8R8A8_SRGB };

		pipelineDesc.inputDesc.resize( 2 );
		pipelineDesc.inputDesc[ 0 ] = { "POSITION", EGPIResourceFormat::R32G32B32_Float, GPIInputClass_PerVertex, 0 };
		pipelineDesc.inputDesc[ 1 ] = { "TEXCOORD", EGPIResourceFormat::R32G32_Float, GPIInputClass_PerVertex, 2 };

		pipelineDesc.vertexShader.hash = 7;
		pipelineDesc.vertexShader.type = ShaderType_VertexShader;
		pipelineDesc.vertexShader.file = "Engine/Shader/Lighting.hlsl";
		pipelineDesc.vertexShader.entry = "VS_LightCombine";

		pipelineDesc.pixelShader.hash = 8;
		pipelineDesc.pixelShader.type = ShaderType_PixelShader;
		pipelineDesc.pixelShader.file = "Engine/Shader/Lighting.hlsl";
		pipelineDesc.pixelShader.entry = "PS_LightCombine";

		pipelineDesc.numCBVs = 1;
		pipelineDesc.numSRVs = 0;

		AtomicEngine::GetGPI()->CreatePipelineState( pipelineDesc );
	}
	//AtomicEngine::GetGPI()->BindConstantBuffer( pipelineDesc, sceneViewComp.constBufferID, 0 );

	/*AtomicEngine::GetGPI()->SetPipelineState( pipelineDesc );

	static IVertexBufferRef positionBuffer = nullptr;
	static IVertexBufferRef uvBuffer = nullptr;
	static IIndexBufferRef indexBuffer = nullptr;

	if( !positionBuffer )
	{
		static StaticMesh staticMesh = SampleMesh::GetQuad();

		positionBuffer = AtomicEngine::GetGPI()->CreateVertexBuffer( staticMesh.GetPositionPtr(), staticMesh.GetPositionStride(), staticMesh.GetPositionByteSize() );
		uvBuffer = AtomicEngine::GetGPI()->CreateVertexBuffer( staticMesh.GetUVPtr(), staticMesh.GetUVStride(), staticMesh.GetUVByteSize() );
		indexBuffer = AtomicEngine::GetGPI()->CreateIndexBuffer( staticMesh.GetIndexPtr( 0 ), staticMesh.GetIndexByteSize( 0 ) );
	}

	AtomicEngine::GetGPI()->Render( positionBuffer.get(), uvBuffer.get(), nullptr, indexBuffer.get() );

	AtomicEngine::GetGPI()->ExecuteCommandList();*/
}
