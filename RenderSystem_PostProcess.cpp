#include "stdafx.h"
#include "RenderSystem.h"
#include "AtomicEngine.h"
#include "GPI.h"
#include "Vector.h"
#include "StaticMesh.h"
#include "GPIPipeline.h"

std::vector<Vec3> position = {
	// Upper Left
	{ -1.0f, 1.0f, 0 },
	// Upper Right
	{ 1.0f, 1.0f, 0 },
	// Bottom right
	{ 1.0f, -1.0f, 0 },
	// Bottom left
	{ -1.0f, -1.0f, 0 }
};

std::vector<Vec2> uv = {
	// Upper Left
	{ 0, 0 },
	// Upper Right
	{ 1, 0 },
	// Bottom right
	{ 1, 1 },
	// Bottom left
	{ 0, 1 }
};

std::vector<std::vector<uint32>> indices = {
	{ 0, 1, 2, 2, 3, 0 }
};

void RenderSystem::PostProcess( std::array<std::unique_ptr<struct IComponentRegistry>, NUM_COMPONENT_MAX>& componentRegistry )
{
	static GPIPipelineStateDesc pipelineDesc{};
	if( pipelineDesc.hash == 0 )
	{
		pipelineDesc.hash = 2;
		pipelineDesc.bRenderSwapChainBuffer = true;
		pipelineDesc.numRenderTargets = 1;
		pipelineDesc.pipelineType = PipelineType_Graphics;

		pipelineDesc.inputDesc.resize( 2 );
		pipelineDesc.inputDesc[ 0 ] = { "POSITION", GPIDataFormat_R32G32B32_Float, GPIInputClass_PerVertex, 0 };
		pipelineDesc.inputDesc[ 1 ] = { "TEXCOORD", GPIDataFormat_R32G32_Float, GPIInputClass_PerVertex, 2 };

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
		static StaticMesh staticMesh;
		staticMesh.position = position;
		staticMesh.uv = uv;
		staticMesh.indices = indices;

		positionBuffer = AtomicEngine::GetGPI()->CreateVertexBuffer( staticMesh.GetPositionPtr(), staticMesh.GetPositionStride(), staticMesh.GetPositionByteSize() );
		uvBuffer = AtomicEngine::GetGPI()->CreateVertexBuffer( staticMesh.GetUVPtr(), staticMesh.GetUVStride(), staticMesh.GetUVByteSize() );
		indexBuffer = AtomicEngine::GetGPI()->CreateIndexBuffer( staticMesh.GetIndexPtr( 0 ), staticMesh.GetIndexByteSize( 0 ) );
	}

	AtomicEngine::GetGPI()->Render( positionBuffer.get(), uvBuffer.get(), nullptr, indexBuffer.get() );

	AtomicEngine::GetGPI()->FlushPipelineState();
}