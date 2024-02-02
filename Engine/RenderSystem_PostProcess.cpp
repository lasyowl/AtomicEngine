#include "RenderSystem.h"
#include "AtomicEngine.h"
#include <GPI/GPI.h>
#include <GPI/GPIPipeline.h>
#include <Core/Vector.h>
#include "StaticMesh.h"
#include "SampleMesh.h"

void RenderSystem::PostProcess( std::array<std::unique_ptr<IComponentRegistry>, NUM_COMPONENT_MAX>& componentRegistry )
{
	return;

	/*static GPIPipelineStateDesc pipelineDesc{};
	if( pipelineDesc.id == 0 )
	{
		pipelineDesc.id = 2;
		pipelineDesc.pipelineType = PipelineType_Graphics;
		pipelineDesc.bRenderSwapChainBuffer = true;
		pipelineDesc.bBindDepth = false;

		pipelineDesc.rtvFormats = { EGPIResourceFormat::B8G8R8A8_SRGB, };

		pipelineDesc.inputDesc.resize( 2 );
		pipelineDesc.inputDesc[ 0 ] = { "POSITION", EGPIResourceFormat::R32G32B32_Float, GPIInputClass_PerVertex, 0 };
		pipelineDesc.inputDesc[ 1 ] = { "TEXCOORD", EGPIResourceFormat::R32G32_Float, GPIInputClass_PerVertex, 2 };

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

	AtomicEngine::GetGPI()->ExecuteCommandList();*/
}