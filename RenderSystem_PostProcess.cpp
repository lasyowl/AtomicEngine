#include "stdafx.h"
#include "RenderSystem.h"
#include "AtomicEngine.h"
#include "GPI.h"
#include "Vector.h"
#include "StaticMesh.h"
#include "GPIPipeline.h"
#include "SampleMesh.h"

void RenderSystem::PostProcess( std::array<std::unique_ptr<struct IComponentRegistry>, NUM_COMPONENT_MAX>& componentRegistry )
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
		pipelineDesc.vertexShader.file = "Engine/Shader/PostProcess.hlsl";
		pipelineDesc.vertexShader.entry = "VS_main";

		pipelineDesc.pixelShader.hash = 4;
		pipelineDesc.pixelShader.type = ShaderType_PixelShader;
		pipelineDesc.pixelShader.file = "Engine/Shader/PostProcess.hlsl";
		pipelineDesc.pixelShader.entry = "PS_main";

		AtomicEngine::GetGPI()->CreatePipelineState( pipelineDesc );
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

	AtomicEngine::GetGPI()->Render( positionBuffer.get(), uvBuffer.get(), nullptr, indexBuffer.get() );

	AtomicEngine::GetGPI()->FlushPipelineState();
}