#include <Engine/RenderSystem.h>
#include <Engine/AtomicEngine.h>
#include <Engine/StaticMesh.h>
#include <GPI/GPI.h>
#include <GPI/GPIPipeline.h>
#include <GPI/GPIUtility.h>
#include <Core/Math.h>
#include <Core/IntVector.h>
#include <Engine/TransformComponent.h>
#include <Engine/PrimitiveComponent.h>
#include <Engine/AssetLoader.h>
#include <Engine/Texture.h>

void RenderSystem::RayTracingTest( std::array<std::unique_ptr<IComponentRegistry>, NUM_COMPONENT_MAX>& componentRegistry )
{
	ComponentRegistry<TransformComponent>* transformCompReg = GetRegistry<TransformComponent>( componentRegistry );
	ComponentRegistry<PrimitiveComponent>* renderCompReg = GetRegistry<PrimitiveComponent>( componentRegistry );
	if( !transformCompReg || !renderCompReg )
	{
		return;
	}

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

	static IGPIRayTraceTopLevelASRef topLevelAS = nullptr;
	static IGPIShaderResourceViewRef testNormalSRV = nullptr;
	static IGPIShaderResourceViewRef testIndexSRV = nullptr;
	static IGPIShaderResourceViewRef testInstanceContextSRV = nullptr;
	static IGPIShaderResourceViewRef testMaterialSRV = nullptr;
	static IGPIDescriptorTableViewRef descTableView = nullptr;
	static IGPIResourceRef testTexResource = nullptr;
	if( !topLevelAS )
	{
		uint32 testNormalResourceByteSize = 0;
		uint32 testIndexResourceByteSize = 0;
		std::vector<IGPIRayTraceBottomLevelASRef> blas;

		for( Entity entity = 0; entity < NUM_ENTITY_MAX; ++entity )
		{
			if( !renderCompReg->HasComponent( entity ) )
			{
				continue;
			}

			PrimitiveComponent& primitiveComp = renderCompReg->GetComponent( entity );
			TransformComponent& transformComp = transformCompReg->GetComponent( entity );

			if( !primitiveComp.staticMeshGroup ) continue;

			StaticMesh& staticMesh = primitiveComp.staticMeshGroup->meshes[ 0 ];
			testNormalResourceByteSize += staticMesh.pipelineInput.vbv[ 1 ]->size;
			testIndexResourceByteSize += staticMesh.pipelineInput.ibv[ 0 ]->size;

			GPIRayTraceBottomLevelASDesc bottomLevelASDesc{};
			bottomLevelASDesc.transform = AEMath::GetTransposedMatrix( AEMath::GetScaleMatrix( transformComp.scale ) * AEMath::GetRotationMatrix( transformComp.rotation ) * AEMath::GetTranslateMatrix( transformComp.position ) );
			IGPIRayTraceBottomLevelASRef bottomLevelAS = AtomicEngine::GetGPI()->CreateRayTraceBottomLevelAS( bottomLevelASDesc, *staticMesh.pipelineInput.vbv[ 0 ], *staticMesh.pipelineInput.ibv[ 0 ] );
			blas.push_back( bottomLevelAS );
		}

		GPIRayTraceTopLevelASDesc topLevelASDesc{};
		topLevelAS = AtomicEngine::GetGPI()->CreateRayTraceTopLevelAS( blas, topLevelASDesc );

		/* Create resource */
		GPIResourceDesc resourceDesc{};
		resourceDesc.dimension = EGPIResourceDimension::Buffer;
		resourceDesc.format = EGPIResourceFormat::Unknown;
		resourceDesc.width = testNormalResourceByteSize;
		resourceDesc.height = 1;
		resourceDesc.depth = 1;
		resourceDesc.numMips = 1;
		resourceDesc.flags = GPIResourceFlag_None;
		resourceDesc.initialState = GPIResourceState_AllShaderResource;
		topLevelAS->normalResource = AtomicEngine::GetGPI()->CreateResource( resourceDesc );

		resourceDesc.width = testIndexResourceByteSize;
		topLevelAS->indexResource = AtomicEngine::GetGPI()->CreateResource( resourceDesc );

		struct RayTraceInstanceContext
		{
			Mat4x4 matRotation;
			uint32 normalResourceOffset;
			uint32 indexResourceOffset;
			// 8Byte padding
		};

		uint32 normalResourceOffset = 0;
		uint32 indexResourceOffset = 0;
		std::vector<RayTraceInstanceContext> instanceContexts;
		std::vector<PBRMaterialConstant> materialConstants;
		instanceContexts.reserve( blas.size() );
		materialConstants.reserve( blas.size() );

		for( Entity entity = 0; entity < NUM_ENTITY_MAX; ++entity )
		{
			if( !renderCompReg->HasComponent( entity ) )
			{
				continue;
			}

			PrimitiveComponent& primitiveComp = renderCompReg->GetComponent( entity );
			TransformComponent& transformComp = transformCompReg->GetComponent( entity );

			if( !primitiveComp.staticMeshGroup ) continue;

			StaticMesh& staticMesh = primitiveComp.staticMeshGroup->meshes[ 0 ];

			AtomicEngine::GetGPI()->CopyBufferRegion( *topLevelAS->normalResource, *staticMesh.normalResource, normalResourceOffset, staticMesh.pipelineInput.vbv[ 1 ]->size );
			AtomicEngine::GetGPI()->CopyBufferRegion( *topLevelAS->indexResource, *staticMesh.indexResource[ 0 ], indexResourceOffset, staticMesh.pipelineInput.ibv[ 0 ]->size );

			RayTraceInstanceContext instanceContext;
			instanceContext.matRotation = AEMath::GetTransposedMatrix( AEMath::GetRotationMatrix( transformComp.rotation ) );
			instanceContext.normalResourceOffset = normalResourceOffset / sizeof( Vec3 );
			instanceContext.indexResourceOffset = indexResourceOffset / sizeof( uint32 );
			instanceContexts.emplace_back( instanceContext );
			materialConstants.emplace_back( primitiveComp.material.constants );
			normalResourceOffset += staticMesh.pipelineInput.vbv[ 1 ]->size;
			indexResourceOffset += staticMesh.pipelineInput.ibv[ 0 ]->size;
		}

		resourceDesc.width = instanceContexts.size() * sizeof( RayTraceInstanceContext );
		topLevelAS->instanceContextResource = AtomicEngine::GetGPI()->CreateResource( resourceDesc, instanceContexts.data(), resourceDesc.width );

		resourceDesc.width = materialConstants.size() * sizeof( PBRMaterialConstant );
		topLevelAS->materialResource = AtomicEngine::GetGPI()->CreateResource( resourceDesc, materialConstants.data(), resourceDesc.width );

		GPIShaderResourceViewDesc srvDesc{};
		srvDesc.format = EGPIResourceFormat::R32G32B32_Float;
		srvDesc.dimension = EGPIResourceDimension::Buffer;
		srvDesc.numElements = testNormalResourceByteSize / sizeof( Vec3 );
		testNormalSRV = AtomicEngine::GetGPI()->CreateShaderResourceView( *topLevelAS->normalResource, srvDesc );

		srvDesc.format = EGPIResourceFormat::R32_Uint;
		srvDesc.numElements = testIndexResourceByteSize / sizeof( uint32 );
		testIndexSRV = AtomicEngine::GetGPI()->CreateShaderResourceView( *topLevelAS->indexResource, srvDesc );

		srvDesc.format = EGPIResourceFormat::Unknown;
		srvDesc.numElements = instanceContexts.size();
		srvDesc.byteStride = sizeof( RayTraceInstanceContext );
		testInstanceContextSRV = AtomicEngine::GetGPI()->CreateShaderResourceView( *topLevelAS->instanceContextResource, srvDesc );

		srvDesc.format = EGPIResourceFormat::Unknown;
		srvDesc.numElements = materialConstants.size();
		srvDesc.byteStride = sizeof( PBRMaterialConstant );
		testMaterialSRV = AtomicEngine::GetGPI()->CreateShaderResourceView( *topLevelAS->materialResource, srvDesc );

		// temp texture
		TextureDataRef baseColorTexture = AssetLoader::LoadTextureData( "../Resource/brick-wall/brick-wall_albedo.png" );
		GPIResourceDesc desc{};
		desc.name = L"testpng";
		desc.dimension = EGPIResourceDimension::Texture2D;
		desc.format = EGPIResourceFormat::B8G8R8A8;
		desc.width = baseColorTexture->width;
		desc.height = baseColorTexture->height;
		desc.depth = 1;
		desc.numMips = 1;
		desc.flags = GPIResourceFlag_None;
		desc.initialState = GPIResourceState_AllShaderResource;
		testTexResource = AtomicEngine::GetGPI()->CreateResource( desc );
		AtomicEngine::GetGPI()->UpdateTextureData( *testTexResource, baseColorTexture->data.data(), baseColorTexture->width, baseColorTexture->height );

		std::vector<const IGPIResource*> resources = 
		{
			testTexResource.get(), _sceneLightResource.get()
		};
		std::vector<GPIConstantBufferViewDesc> cbvDescs;
		std::vector<GPIShaderResourceViewDesc> srvDescs =
		{
			{ EGPIResourceFormat::B8G8R8A8, EGPIResourceDimension::Texture2D, 0, 0 }
		};
		std::vector<GPIUnorderedAccessViewDesc> uavDescs =
		{
			{ EGPIResourceFormat::B8G8R8A8, EGPIResourceDimension::Texture2D }
		};
		descTableView = AtomicEngine::GetGPI()->CreateDescriptorTableView( resources, cbvDescs, srvDescs, uavDescs );
	}

	AtomicEngine::GetGPI()->RayTrace( pipelineDesc, topLevelAS, descTableView, testNormalSRV, testIndexSRV, testInstanceContextSRV, testMaterialSRV );
}