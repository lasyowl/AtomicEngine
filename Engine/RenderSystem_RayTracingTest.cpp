#include <Engine/RenderSystem.h>
#include <Engine/AtomicEngine.h>
#include <Engine/StaticMesh.h>
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

	static IGPIRayTraceBottomLevelASRef bottomLevelAS = nullptr;
	static IGPIRayTraceTopLevelASRef topLevelAS = nullptr;
	if( !bottomLevelAS )
	{
		StaticMeshGroupRef& staticMeshGroup = StaticMeshCache::FindStaticMeshGroup( "quad" );
		StaticMesh& staticMesh = staticMeshGroup->meshes[ 0 ];

		GPIRayTraceBottomLevelASDesc bottomLevelASDesc{};
		bottomLevelAS = AtomicEngine::GetGPI()->CreateRayTraceBottomLevelAS( bottomLevelASDesc, *staticMesh.pipelineInput.vbv[ 0 ], *staticMesh.pipelineInput.ibv[ 0 ] );

		std::vector<IGPIRayTraceBottomLevelASRef> bottomLevelASs = { bottomLevelAS };
		GPIRayTraceTopLevelASDesc topLevelASDesc{};
		topLevelAS = AtomicEngine::GetGPI()->CreateRayTraceTopLevelAS( bottomLevelASs, topLevelASDesc );
	}

	AtomicEngine::GetGPI()->RayTrace( pipelineDesc, topLevelAS );
}