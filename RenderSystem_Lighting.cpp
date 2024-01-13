#include "stdafx.h"
#include "RenderSystem.h"
#include "AtomicEngine.h"
#include "GPI.h"
#include "GPIPipeline.h"
#include "StaticMesh.h"
#include "SampleMesh.h"

void RenderSystem::Lighting( std::array<std::unique_ptr<IComponentRegistry>, NUM_COMPONENT_MAX>& componentRegistry )
{
	static GPIPipelineStateDesc pipelineDesc{};
	if( pipelineDesc.hash == 0 )
	{
		pipelineDesc.hash = 2;
		pipelineDesc.pipelineType = PipelineType_Graphics;
		pipelineDesc.bRenderSwapChainBuffer = true;
		pipelineDesc.bWriteDepth = false;
		pipelineDesc.numRenderTargets = 1;
		pipelineDesc.renderTargetDesc.resize( pipelineDesc.numRenderTargets );
		pipelineDesc.renderTargetDesc[ 0 ].format = GPIBufferFormat_B8G8R8A8_SRGB;

		pipelineDesc.inputDesc.resize( 2 );
		pipelineDesc.inputDesc[ 0 ] = { "POSITION", GPIBufferFormat_R32G32B32_Float, GPIInputClass_PerVertex, 0 };
		pipelineDesc.inputDesc[ 1 ] = { "TEXCOORD", GPIBufferFormat_R32G32_Float, GPIInputClass_PerVertex, 2 };

		pipelineDesc.vertexShader.hash = 3;
		pipelineDesc.vertexShader.type = ShaderType_VertexShader;
		pipelineDesc.vertexShader.file = "Engine/Shader/Lighting.hlsl";
		pipelineDesc.vertexShader.entry = "VS_main";

		pipelineDesc.pixelShader.hash = 4;
		pipelineDesc.pixelShader.type = ShaderType_PixelShader;
		pipelineDesc.pixelShader.file = "Engine/Shader/Lighting.hlsl";
		pipelineDesc.pixelShader.entry = "PS_main";

		pipelineDesc.numConstantBuffers = 1;

		pipelineDesc.numResources = 2;

		AtomicEngine::GetGPI()->CreatePipelineState( pipelineDesc );

		struct PointLightResourceBuffer
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

		AtomicEngine::GetGPI()->BindResourceBuffer( pipelineDesc, pointLightBuffer.positionBuffer.get(), 0);
	}
	AtomicEngine::GetGPI()->SetPipelineState( pipelineDesc );

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

	AtomicEngine::GetGPI()->FlushPipelineState();
}