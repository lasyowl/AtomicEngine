#include "RenderSystem.h"
#include "AtomicEngine.h"
#include <RHI/RHI.h>
#include <RHI/RHIPipeline.h>
#include <RHI/RHIUtility.h>
#include <Core/Math.h>
#include "StaticMesh.h"
#include "SampleMesh.h"
#include "LightComponent.h"
#include "TransformComponent.h"
#include "PrimitiveComponent.h"

void RenderSystem::DirectionalLight( std::array<std::unique_ptr<IComponentRegistry>, NUM_COMPONENT_MAX>& componentRegistry )
{
	static RHIPipelineStateDesc pipelineDesc{};
	static IRHIPipelineRef pipeline = nullptr;
	if( !pipeline )
	{
		pipelineDesc.id = 1;
		pipelineDesc.pipelineType = PipelineType_Graphics;
		pipelineDesc.bBindDepth = false;

		pipelineDesc.rtvFormats = { ERHIResourceFormat::B8G8R8A8_SRGB };

		pipelineDesc.inputDesc = {
			{ "POSITION", ERHIResourceFormat::R32G32B32_Float, RHIInputClass_PerVertex, 0 },
			{ "TEXCOORD", ERHIResourceFormat::R32G32_Float, RHIInputClass_PerVertex, 2 }
		};

		pipelineDesc.vertexShader.hash = 2;
		pipelineDesc.vertexShader.type = ShaderType_VertexShader;
		pipelineDesc.vertexShader.file = "../Engine/Shader/Lighting.hlsl";
		pipelineDesc.vertexShader.entry = "VS_DirectionalLight";

		pipelineDesc.pixelShader.hash = 3;
		pipelineDesc.pixelShader.type = ShaderType_PixelShader;
		pipelineDesc.pixelShader.file = "../Engine/Shader/Lighting.hlsl";
		pipelineDesc.pixelShader.entry = "PS_DirectionalLight";

		pipelineDesc.numCBVs = 1;
		pipelineDesc.numTextures = { 4 };

		pipeline = AtomicEngine::GetRHI()->CreatePipelineState( pipelineDesc );
	}

	if( !_sceneLightResource )
	{
		const IVec2 windowSize = AtomicEngine::GetRHI()->GetWindowSize();
		const RHIResourceDesc sceneLightDesc = RHIUtil::GetRenderTargetResourceDesc( L"SceneLight", windowSize );
		_sceneLightResource = AtomicEngine::GetRHI()->CreateResource( sceneLightDesc );

		RHIRenderTargetViewDesc rtvDesc{};
		rtvDesc.format = ERHIResourceFormat::B8G8R8A8_SRGB;
		rtvDesc.dimension = ERHIResourceDimension::Texture2D;

		_sceneLightRTV = AtomicEngine::GetRHI()->CreateRenderTargetView( *_sceneLightResource, rtvDesc );

		RHIShaderResourceViewDesc srvDesc{};
		srvDesc.format = ERHIResourceFormat::B8G8R8A8_SRGB;
		srvDesc.dimension = ERHIResourceDimension::Texture2D;

		std::vector<const IRHIResource*> textureResources = {
			_sceneLightResource.get()
		};
		std::vector<RHIShaderResourceViewDesc> textureDescs = {
			srvDesc
		};
		_sceneLightTextureViewTable = AtomicEngine::GetRHI()->CreateTextureViewTable( textureResources, textureDescs );

		RHIUnorderedAccessViewDesc uavDesc{};
		uavDesc.format = ERHIResourceFormat::B8G8R8A8;
		uavDesc.dimension = ERHIResourceDimension::Texture2D;
		_sceneLightUAV = AtomicEngine::GetRHI()->CreateUnorderedAccessView( *_sceneLightResource, uavDesc, false );

		uavDesc.format = ERHIResourceFormat::R32_Uint;
		_sceneLightUAVSH = AtomicEngine::GetRHI()->CreateUnorderedAccessView( *_sceneLightResource, uavDesc, true );
	}

	{// @TODO: move to somewhere makes sense
		AtomicEngine::GetRHI()->BindRenderTargetView( *pipeline, *_sceneLightRTV, 0 );

		AtomicEngine::GetRHI()->BindConstantBufferView( *pipeline, *_viewCBV, 0 );

		AtomicEngine::GetRHI()->BindTextureViewTable( *pipeline, *_gBufferTextureViewTable, 0 );
	}

	AtomicEngine::GetRHI()->ClearRenderTarget( *_sceneLightUAVSH );

	AtomicEngine::GetRHI()->SetPipelineState( pipelineDesc );

	for( const StaticMesh& mesh : StaticMeshCache::FindStaticMeshGroup( "quad" )->meshes )
	{
		AtomicEngine::GetRHI()->Render( mesh.pipelineInput );
	}

	AtomicEngine::GetRHI()->ExecuteCommandList();
}

void RenderSystem::PointLight( std::array<std::unique_ptr<IComponentRegistry>, NUM_COMPONENT_MAX>& componentRegistry )
{
	ComponentRegistry<TransformComponent>* transformCompReg = GetRegistry<TransformComponent>( componentRegistry );
	ComponentRegistry<PrimitiveComponent>* renderCompReg = GetRegistry<PrimitiveComponent>( componentRegistry );
	ComponentRegistry<LightComponent>* lightCompReg = GetRegistry<LightComponent>( componentRegistry );

	if( !transformCompReg || !renderCompReg || !lightCompReg )
	{
		return;
	}

	static RHIPipelineStateDesc pipelineDesc{};
	static IRHIPipelineRef pipeline = nullptr;
	if( !pipeline )
	{
		pipelineDesc.id = 2;
		pipelineDesc.pipelineType = PipelineType_Graphics;
		pipelineDesc.bBindDepth = false;
		pipelineDesc.bEnableBlend = true;

		pipelineDesc.rtvFormats = { ERHIResourceFormat::B8G8R8A8_SRGB };

		pipelineDesc.inputDesc = {
			{ "POSITION", ERHIResourceFormat::R32G32B32_Float, RHIInputClass_PerVertex, 0 },
			{ "TEXCOORD", ERHIResourceFormat::R32G32_Float, RHIInputClass_PerVertex, 2 }
		};

		pipelineDesc.vertexShader.hash = 4;
		pipelineDesc.vertexShader.type = ShaderType_VertexShader;
		pipelineDesc.vertexShader.file = "../Engine/Shader/Lighting.hlsl";
		pipelineDesc.vertexShader.entry = "VS_PointLight";

		pipelineDesc.pixelShader.hash = 5;
		pipelineDesc.pixelShader.type = ShaderType_PixelShader;
		pipelineDesc.pixelShader.file = "../Engine/Shader/Lighting.hlsl";
		pipelineDesc.pixelShader.entry = "PS_PointLight";

		pipelineDesc.numCBVs = 2;
		pipelineDesc.numTextures = { 4 };

		pipeline = AtomicEngine::GetRHI()->CreatePipelineState( pipelineDesc );
	}

	{// @TODO: move to somewhere makes sense
		AtomicEngine::GetRHI()->BindRenderTargetView( *pipeline, *_sceneLightRTV, 0 );

		AtomicEngine::GetRHI()->BindTextureViewTable( *pipeline, *_gBufferTextureViewTable, 0 );
	}

	__declspec( align( 256 ) )
		struct LightConstantBuffer
	{
		Mat4x4 matModel;
		Vec3 origin;
		float intensity;
	};

	static IRHIResourceRef lightCBResource = nullptr;
	static IRHIConstantBufferViewRef lightCBV;
	if( !lightCBResource )
	{
		const RHIResourceDesc cbDesc = RHIUtil::GetConstantBufferResourceDesc( L"LightConstant", sizeof( LightConstantBuffer ) );
		lightCBResource = AtomicEngine::GetRHI()->CreateResource( cbDesc );

		RHIConstantBufferViewDesc cbvDesc{};
		cbvDesc.sizeInBytes = sizeof( LightConstantBuffer );

		lightCBV = AtomicEngine::GetRHI()->CreateConstantBufferView( *lightCBResource, cbvDesc );
	}

	{// @TODO: move to somewhere makes sense
		AtomicEngine::GetRHI()->BindConstantBufferView( *pipeline, *_viewCBV, 0 );
		AtomicEngine::GetRHI()->BindConstantBufferView( *pipeline, *lightCBV, 1 );
	}

	const StaticMeshGroupRef& meshGroup = StaticMeshCache::FindStaticMeshGroup( "sphere" );

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
		float lightIntensity = 4.0f;

		LightConstantBuffer lightBuffer;
		lightBuffer.matModel = AEMath::GetTransposedMatrix( AEMath::GetScaleMatrix( Vec3( lightIntensity, lightIntensity, lightIntensity ) ) * AEMath::GetTranslateMatrix( earlyTransform ) * AEMath::GetRotationMatrix( transformComp.rotation ) * AEMath::GetTranslateMatrix( transformComp.position ) );
		lightBuffer.origin = transformComp.position;
		lightBuffer.intensity = lightIntensity;

		AtomicEngine::GetRHI()->UpdateResourceData( *lightCBResource, &lightBuffer, sizeof( lightBuffer ) );

		AtomicEngine::GetRHI()->SetPipelineState( pipelineDesc );

		for( const StaticMesh& mesh : meshGroup->meshes )
		{
			AtomicEngine::GetRHI()->Render( mesh.pipelineInput );
		}

		AtomicEngine::GetRHI()->ExecuteCommandList();
	}
}

void RenderSystem::LightCombine( std::array<std::unique_ptr<IComponentRegistry>, NUM_COMPONENT_MAX>& componentRegistry )
{
	static RHIPipelineStateDesc pipelineDesc{};
	static IRHIPipelineRef pipeline = nullptr;
	if( !pipeline )
	{
		pipelineDesc.id = 3;
		pipelineDesc.pipelineType = PipelineType_Graphics;
		pipelineDesc.bBindDepth = false;

		pipelineDesc.rtvFormats = { ERHIResourceFormat::B8G8R8A8_SRGB };

		pipelineDesc.inputDesc = {
			{ "POSITION", ERHIResourceFormat::R32G32B32_Float, RHIInputClass_PerVertex, 0 },
			{ "TEXCOORD", ERHIResourceFormat::R32G32_Float, RHIInputClass_PerVertex, 2 }
		};

		pipelineDesc.vertexShader.hash = 6;
		pipelineDesc.vertexShader.type = ShaderType_VertexShader;
		pipelineDesc.vertexShader.file = "../Engine/Shader/Lighting.hlsl";
		pipelineDesc.vertexShader.entry = "VS_LightCombine";

		pipelineDesc.pixelShader.hash = 7;
		pipelineDesc.pixelShader.type = ShaderType_PixelShader;
		pipelineDesc.pixelShader.file = "../Engine/Shader/Lighting.hlsl";
		pipelineDesc.pixelShader.entry = "PS_LightCombine";

		pipelineDesc.numTextures = { 4, 1 };

		pipeline = AtomicEngine::GetRHI()->CreatePipelineState( pipelineDesc );
	}

	{// @TODO: move to somewhere makes sense
		uint32 swapChainIndex = AtomicEngine::GetRHI()->GetSwapChainCurrentIndex();
		IRHIRenderTargetViewRef& swapChainRTV = _swapChainRTV[ swapChainIndex ];
		AtomicEngine::GetRHI()->BindRenderTargetView( *pipeline, *_swapChainRTV[ swapChainIndex ], 0 );

		AtomicEngine::GetRHI()->BindTextureViewTable( *pipeline, *_gBufferTextureViewTable, 0 );
		AtomicEngine::GetRHI()->BindTextureViewTable( *pipeline, *_sceneLightTextureViewTable, 1 );
	}

	AtomicEngine::GetRHI()->SetPipelineState( pipelineDesc );

	for( const StaticMesh& mesh : StaticMeshCache::FindStaticMeshGroup( "quad" )->meshes )
	{
		AtomicEngine::GetRHI()->Render( mesh.pipelineInput );
	}

	AtomicEngine::GetRHI()->ExecuteCommandList();
}
