#include <Engine/RenderSystem.h>
#include <Engine/AtomicEngine.h>
#include <Engine/StaticMesh.h>
#include <GPI/GPI.h>
#include <GPI/GPIPipeline.h>
#include <GPI/GPIUtility.h>

void RenderSystem::RayTracingTest( std::array<std::unique_ptr<IComponentRegistry>, NUM_COMPONENT_MAX>& componentRegistry )
{
	static GPIPipelineStateDesc pipelineDesc{};
	static IGPIPipelineRef pipeline = nullptr;
	if( !pipeline )
	{
		pipelineDesc.id = 4;
		pipelineDesc.pipelineType = PipelineType_RayTrace;

		pipelineDesc.raytraceShader.hash = 8;
		pipelineDesc.raytraceShader.type = ShaderType_RayTraceShader;
		pipelineDesc.raytraceShader.file = "../Engine/Shader/RayTracingTest.hlsl";
		pipelineDesc.raytraceShader.macros = { { "D3D12_SAMPLE_CONSTANT_BUFFER", "1" } };

		pipelineDesc.numCBVs = 1;
		pipelineDesc.numUAVs = 1;

		pipeline = AtomicEngine::GetGPI()->CreatePipelineState( pipelineDesc );

		AtomicEngine::GetGPI()->BindConstantBufferView( *pipeline, *_viewCBV, 0 );
		AtomicEngine::GetGPI()->BindUnorderedAccessView( *pipeline, *_sceneLightUAV, 0 );
	}

	static IGPIRayTraceBottomLevelASRef bottomLevelAS = nullptr;
	static IGPIRayTraceTopLevelASRef topLevelAS = nullptr;
	static IGPIResourceRef testResource = nullptr;
	static IGPIShaderResourceViewRef testNormalSRV = nullptr;
	static IGPIShaderResourceViewRef testIndexSRV = nullptr;
	if( !bottomLevelAS )
	{
		StaticMeshGroupRef& staticMeshGroup = StaticMeshCache::FindStaticMeshGroup( "teapot" );
		StaticMesh& staticMesh = staticMeshGroup->meshes[ 0 ];

		GPIRayTraceBottomLevelASDesc bottomLevelASDesc{};
		bottomLevelAS = AtomicEngine::GetGPI()->CreateRayTraceBottomLevelAS( bottomLevelASDesc, *staticMesh.pipelineInput.vbv[ 0 ], *staticMesh.pipelineInput.ibv[ 0 ] );

		std::vector<IGPIRayTraceBottomLevelASRef> bottomLevelASs = { bottomLevelAS };
		GPIRayTraceTopLevelASDesc topLevelASDesc{};
		topLevelAS = AtomicEngine::GetGPI()->CreateRayTraceTopLevelAS( bottomLevelASs, topLevelASDesc );

		GPIShaderResourceViewDesc srvDesc{};
		srvDesc.format = EGPIResourceFormat::R32G32B32_Float;
		srvDesc.dimension = EGPIResourceDimension::Buffer;
		srvDesc.numElements = staticMesh.pipelineInput.vbv[ 1 ]->size / staticMesh.pipelineInput.vbv[ 1 ]->stride;
		testNormalSRV = AtomicEngine::GetGPI()->CreateShaderResourceView( *staticMesh.normalResource, srvDesc );
		srvDesc.format = EGPIResourceFormat::R32_Uint;
		srvDesc.numElements = staticMesh.pipelineInput.ibv[ 0 ]->size / 4;
		testIndexSRV = AtomicEngine::GetGPI()->CreateShaderResourceView( *staticMesh.indexResource[ 0 ], srvDesc );
	}

	AtomicEngine::GetGPI()->RayTrace( pipelineDesc, topLevelAS, testNormalSRV, testIndexSRV );
}