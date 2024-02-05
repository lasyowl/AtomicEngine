#include <Engine/RenderSystem.h>
#include <Engine/AtomicEngine.h>
#include <GPI/GPI.h>
#include <GPI/GPIPipeline.h>

void RenderSystem::RayTracingTest( std::array<std::unique_ptr<IComponentRegistry>, NUM_COMPONENT_MAX>& componentRegistry )
{
	static GPIPipelineStateDesc pipelineDesc{};
	static IGPIPipelineRef pipeline = nullptr;
	if( !pipeline )
	{
		pipelineDesc.id = 4;
		pipelineDesc.pipelineType = PipelineType_RayTrace;

		pipelineDesc.raytraceShader.hash = 0;
		pipelineDesc.raytraceShader.type = ShaderType_RayTraceShader;
		pipelineDesc.raytraceShader.file = "../Engine/Shader/RayTracingTest.hlsl";
		pipelineDesc.raytraceShader.entry = "RayGeneration";
		pipelineDesc.raytraceShader.macros.resize( 1 );
		pipelineDesc.raytraceShader.macros[ 0 ] = { "D3D12_SAMPLE_CONSTANT_BUFFER", "1" };

		pipelineDesc.numUAVs = 1;

		pipeline = AtomicEngine::GetGPI()->CreatePipelineState( pipelineDesc );

		AtomicEngine::GetGPI()->BindUnorderedAccessView( *pipeline, *_sceneLightUAV, 0 );
	}

	AtomicEngine::GetGPI()->RayCast( pipelineDesc );
}