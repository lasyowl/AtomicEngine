#include <Engine/RenderSystem.h>
#include <Engine/AtomicEngine.h>
#include <Engine/StaticMesh.h>
#include <RHI/RHI.h>
#include <RHI/RHIPipeline.h>
#include <RHI/RHIUtility.h>
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

	static RHIPipelineStateDesc pipelineDesc{};
	static IRHIPipelineRef pipeline = nullptr;
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

		pipeline = AtomicEngine::GetRHI()->CreatePipelineState( pipelineDesc );

		AtomicEngine::GetRHI()->BindConstantBufferView( *pipeline, *_viewCBV, 0 );
		AtomicEngine::GetRHI()->BindUnorderedAccessView( *pipeline, *_sceneLightUAV, 0 );
	}

	static IRHIRayTraceTopLevelASRef topLevelAS = nullptr;
	static IRHIShaderResourceViewRef testNormalSRV = nullptr;
	static IRHIShaderResourceViewRef testIndexSRV = nullptr;
	static IRHIShaderResourceViewRef testInstanceContextSRV = nullptr;
	static IRHIShaderResourceViewRef testMaterialSRV = nullptr;
	static IRHIDescriptorTableViewRef descTableView = nullptr;
	static IRHIResourceRef testTexResource = nullptr;
	if( !topLevelAS )
	{
		uint32 testNormalResourceByteSize = 0;
		uint32 testIndexResourceByteSize = 0;
		std::vector<IRHIRayTraceBottomLevelASRef> blas;

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

			RHIRayTraceBottomLevelASDesc bottomLevelASDesc{};
			bottomLevelASDesc.transform = AEMath::GetTransposedMatrix( AEMath::GetScaleMatrix( transformComp.scale ) * AEMath::GetRotationMatrix( transformComp.rotation ) * AEMath::GetTranslateMatrix( transformComp.position ) );
			IRHIRayTraceBottomLevelASRef bottomLevelAS = AtomicEngine::GetRHI()->CreateRayTraceBottomLevelAS( bottomLevelASDesc, *staticMesh.pipelineInput.vbv[ 0 ], *staticMesh.pipelineInput.ibv[ 0 ] );
			blas.push_back( bottomLevelAS );
		}

		RHIRayTraceTopLevelASDesc topLevelASDesc{};
		topLevelAS = AtomicEngine::GetRHI()->CreateRayTraceTopLevelAS( blas, topLevelASDesc );

		/* Create resource */
		RHIResourceDesc resourceDesc{};
		resourceDesc.dimension = ERHIResourceDimension::Buffer;
		resourceDesc.format = ERHIResourceFormat::Unknown;
		resourceDesc.width = testNormalResourceByteSize;
		resourceDesc.height = 1;
		resourceDesc.depth = 1;
		resourceDesc.numMips = 1;
		resourceDesc.flags = RHIResourceFlag_None;
		resourceDesc.initialState = RHIResourceState_AllShaderResource;
		topLevelAS->normalResource = AtomicEngine::GetRHI()->CreateResource( resourceDesc );

		resourceDesc.width = testIndexResourceByteSize;
		topLevelAS->indexResource = AtomicEngine::GetRHI()->CreateResource( resourceDesc );

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

			AtomicEngine::GetRHI()->CopyBufferRegion( *topLevelAS->normalResource, *staticMesh.normalResource, normalResourceOffset, staticMesh.pipelineInput.vbv[ 1 ]->size );
			AtomicEngine::GetRHI()->CopyBufferRegion( *topLevelAS->indexResource, *staticMesh.indexResource[ 0 ], indexResourceOffset, staticMesh.pipelineInput.ibv[ 0 ]->size );

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
		topLevelAS->instanceContextResource = AtomicEngine::GetRHI()->CreateResource( resourceDesc, instanceContexts.data(), resourceDesc.width );

		resourceDesc.width = materialConstants.size() * sizeof( PBRMaterialConstant );
		topLevelAS->materialResource = AtomicEngine::GetRHI()->CreateResource( resourceDesc, materialConstants.data(), resourceDesc.width );

		RHIShaderResourceViewDesc srvDesc{};
		srvDesc.format = ERHIResourceFormat::R32G32B32_Float;
		srvDesc.dimension = ERHIResourceDimension::Buffer;
		srvDesc.numElements = testNormalResourceByteSize / sizeof( Vec3 );
		testNormalSRV = AtomicEngine::GetRHI()->CreateShaderResourceView( *topLevelAS->normalResource, srvDesc );

		srvDesc.format = ERHIResourceFormat::R32_Uint;
		srvDesc.numElements = testIndexResourceByteSize / sizeof( uint32 );
		testIndexSRV = AtomicEngine::GetRHI()->CreateShaderResourceView( *topLevelAS->indexResource, srvDesc );

		srvDesc.format = ERHIResourceFormat::Unknown;
		srvDesc.numElements = instanceContexts.size();
		srvDesc.byteStride = sizeof( RayTraceInstanceContext );
		testInstanceContextSRV = AtomicEngine::GetRHI()->CreateShaderResourceView( *topLevelAS->instanceContextResource, srvDesc );

		srvDesc.format = ERHIResourceFormat::Unknown;
		srvDesc.numElements = materialConstants.size();
		srvDesc.byteStride = sizeof( PBRMaterialConstant );
		testMaterialSRV = AtomicEngine::GetRHI()->CreateShaderResourceView( *topLevelAS->materialResource, srvDesc );

		// temp texture
		TextureDataRef baseColorTexture = AssetLoader::LoadTextureData( "../Resource/brick-wall/brick-wall_albedo.png" );
		RHIResourceDesc desc{};
		desc.name = L"testpng";
		desc.dimension = ERHIResourceDimension::Texture2D;
		desc.format = ERHIResourceFormat::B8G8R8A8;
		desc.width = baseColorTexture->width;
		desc.height = baseColorTexture->height;
		desc.depth = 1;
		desc.numMips = 1;
		desc.flags = RHIResourceFlag_None;
		desc.initialState = RHIResourceState_AllShaderResource;
		testTexResource = AtomicEngine::GetRHI()->CreateResource( desc );
		AtomicEngine::GetRHI()->UpdateTextureData( *testTexResource, baseColorTexture->data.data(), baseColorTexture->width, baseColorTexture->height );

		std::vector<const IRHIResource*> resources = 
		{
			testTexResource.get(), _sceneLightResource.get()
		};
		std::vector<RHIConstantBufferViewDesc> cbvDescs;
		std::vector<RHIShaderResourceViewDesc> srvDescs =
		{
			{ ERHIResourceFormat::B8G8R8A8, ERHIResourceDimension::Texture2D, 0, 0 }
		};
		std::vector<RHIUnorderedAccessViewDesc> uavDescs =
		{
			{ ERHIResourceFormat::B8G8R8A8, ERHIResourceDimension::Texture2D }
		};
		descTableView = AtomicEngine::GetRHI()->CreateDescriptorTableView( resources, cbvDescs, srvDescs, uavDescs );
	}

	AtomicEngine::GetRHI()->RayTrace( pipelineDesc, topLevelAS, descTableView, testNormalSRV, testIndexSRV, testInstanceContextSRV, testMaterialSRV );
}