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
	static IGPIPipelineRef pipeline = nullptr;
	if( !pipeline )
	{
		pipelineDesc.id = 1;
		pipelineDesc.pipelineType = PipelineType_Graphics;
		pipelineDesc.bWriteDepth = false;

		pipelineDesc.rtvFormats = { EGPIResourceFormat::B8G8R8A8_SRGB };

		pipelineDesc.inputDesc.resize( 2 );
		pipelineDesc.inputDesc[ 0 ] = { "POSITION", EGPIResourceFormat::R32G32B32_Float, GPIInputClass_PerVertex, 0 };
		pipelineDesc.inputDesc[ 1 ] = { "TEXCOORD", EGPIResourceFormat::R32G32_Float, GPIInputClass_PerVertex, 1 };

		pipelineDesc.vertexShader.hash = 2;
		pipelineDesc.vertexShader.type = ShaderType_VertexShader;
		pipelineDesc.vertexShader.file = "Engine/Shader/Lighting.hlsl";
		pipelineDesc.vertexShader.entry = "VS_DirectionalLight";

		pipelineDesc.pixelShader.hash = 3;
		pipelineDesc.pixelShader.type = ShaderType_PixelShader;
		pipelineDesc.pixelShader.file = "Engine/Shader/Lighting.hlsl";
		pipelineDesc.pixelShader.entry = "PS_DirectionalLight";

		pipelineDesc.numCBVs = 1;
		pipelineDesc.numTextures = { 4/*, 1*/ };

		pipeline = AtomicEngine::GetGPI()->CreatePipelineState( pipelineDesc );

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

	if( !_sceneLightResource )
	{
		/*GPIResourceDesc sceneLightDesc{};
		sceneLightDesc.dimension = EGPIResourceDimension::Texture2D;
		sceneLightDesc.format = EGPIResourceFormat::B8G8R8A8;
		sceneLightDesc.width = 1920;
		sceneLightDesc.height = 1080;
		sceneLightDesc.depth = 1;
		sceneLightDesc.numMips = 1;
		sceneLightDesc.flags = GPIResourceFlag_AllowRenderTarget | GPIResourceFlag_AllowUnorderedAccess;
		sceneLightDesc.usage = GPIResourceUsage_RenderTarget;
		sceneLightDesc.clearValue.type = EGPIResourceClearValueType::Color;
		sceneLightDesc.clearValue.color = Vec4( 0.0f, 0.0f, 0.0f, 0.0f );

		_sceneLightResource = AtomicEngine::GetGPI()->CreateResource( sceneLightDesc );

		GPIRenderTargetViewDesc rtvDesc{};
		rtvDesc.format = EGPIResourceFormat::B8G8R8A8_SRGB;
		rtvDesc.dimension = EGPIResourceDimension::Texture2D;

		_sceneLightRTV = AtomicEngine::GetGPI()->CreateRenderTargetView( *_sceneLightResource, rtvDesc );

		GPIShaderResourceViewDesc srvDesc{};
		srvDesc.format = EGPIResourceFormat::B8G8R8A8_SRGB;
		srvDesc.dimension = EGPIResourceDimension::Texture2D;

		std::vector<const IGPIResource*> textureResources = {
			_sceneLightResource.get()
		};
		std::vector<GPIShaderResourceViewDesc> textureDescs = {
			srvDesc
		};
		_sceneLightTextureViewTable = AtomicEngine::GetGPI()->CreateTextureViewTable( textureResources, textureDescs );

		GPIUnorderedAccessViewDesc uavDesc{};
		uavDesc.format = EGPIResourceFormat::R32_Uint;
		uavDesc.dimension = EGPIResourceDimension::Texture2D;

		_sceneLightUAV = AtomicEngine::GetGPI()->CreateUnorderedAccessView( *_sceneLightResource, uavDesc, true );*/
	}

	{// @TODO: move to somewhere makes sense
		uint32 swapChainIndex = AtomicEngine::GetGPI()->GetSwapChainCurrentIndex();
		IGPIRenderTargetViewRef& swapChainRTV = _swapChainRTV[ swapChainIndex ];
		AtomicEngine::GetGPI()->BindRenderTargetView( *pipeline, *_swapChainRTV[ swapChainIndex ], 0 );
	}

	{// @TODO: move to somewhere makes sense
		AtomicEngine::GetGPI()->BindConstantBufferView( *pipeline, *_viewCBV, 0 );
	}

	{// @TODO: move to somewhere makes sense
		AtomicEngine::GetGPI()->BindTextureViewTable( *pipeline, *_gBufferTextureViewTable, 0 );
		//AtomicEngine::GetGPI()->BindTextureViewTable( *pipeline, *_sceneLightTextureViewTable, 1 );
	}

	static IGPIResourceRef positionResource;
	static IGPIResourceRef uvResource;
	static IGPIResourceRef indexResource;
	static GPIPipelineInput pipelineInput;

	if( !positionResource )
	{
		StaticMesh staticMesh = SampleMesh::GetQuad();

		GPIResourceDesc desc{};
		desc.dimension = EGPIResourceDimension::Buffer;
		desc.format = EGPIResourceFormat::Unknown;
		//desc.width = size;
		desc.height = 1;
		desc.depth = 1;
		desc.numMips = 1;
		desc.flags = GPIResourceFlag_None;

		pipelineInput.vbv.resize( 2 );
		pipelineInput.ibv.resize( 1 );

		desc.width = staticMesh.GetPositionByteSize();
		positionResource = AtomicEngine::GetGPI()->CreateResource( desc, staticMesh.GetPositionPtr(), staticMesh.GetPositionByteSize() );
		pipelineInput.vbv[ 0 ] = AtomicEngine::GetGPI()->CreateVertexBufferView( *positionResource, staticMesh.GetPositionByteSize(), staticMesh.GetPositionStride() );

		desc.width = staticMesh.GetUVByteSize();
		uvResource = AtomicEngine::GetGPI()->CreateResource( desc, staticMesh.GetUVPtr(), staticMesh.GetUVByteSize() );
		pipelineInput.vbv[ 1 ] = AtomicEngine::GetGPI()->CreateVertexBufferView( *uvResource, staticMesh.GetUVByteSize(), staticMesh.GetUVStride() );

		desc.width = staticMesh.GetIndexByteSize( 0 );
		indexResource = AtomicEngine::GetGPI()->CreateResource( desc, staticMesh.GetIndexPtr( 0 ), staticMesh.GetIndexByteSize( 0 ) );
		pipelineInput.ibv[ 0 ] = AtomicEngine::GetGPI()->CreateIndexBufferView( *indexResource, staticMesh.GetIndexByteSize( 0 ) );
	}

	//AtomicEngine::GetGPI()->ClearRenderTarget( *_sceneLightUAV );

	AtomicEngine::GetGPI()->SetPipelineState( pipelineDesc );

	AtomicEngine::GetGPI()->Render( pipelineInput );

	AtomicEngine::GetGPI()->ExecuteCommandList();
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
		pipelineDesc.bWriteDepth = false;

		pipelineDesc.rtvFormats = { EGPIResourceFormat::B8G8R8A8_SRGB };

		pipelineDesc.inputDesc.resize( 2 );
		pipelineDesc.inputDesc[ 0 ] = { "POSITION", EGPIResourceFormat::R32G32B32_Float, GPIInputClass_PerVertex, 0 };
		pipelineDesc.inputDesc[ 1 ] = { "TEXCOORD", EGPIResourceFormat::R32G32_Float, GPIInputClass_PerVertex, 2 };

		pipelineDesc.vertexShader.hash = 4;
		pipelineDesc.vertexShader.type = ShaderType_VertexShader;
		pipelineDesc.vertexShader.file = "Engine/Shader/Lighting.hlsl";
		pipelineDesc.vertexShader.entry = "VS_PointLight";

		pipelineDesc.pixelShader.hash = 5;
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
