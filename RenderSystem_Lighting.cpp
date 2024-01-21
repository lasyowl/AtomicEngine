#include "stdafx.h"
#include "RenderSystem.h"
#include "AtomicEngine.h"
#include "GPI.h"
#include "GPIPipeline.h"
#include "StaticMesh.h"
#include "SampleMesh.h"
#include "LightComponent.h"
#include "TransformComponent.h"
#include "PrimitiveComponent.h"
#include "Math.h"
#include "GPIUtility.h"

void RenderSystem::DirectionalLight( std::array<std::unique_ptr<IComponentRegistry>, NUM_COMPONENT_MAX>& componentRegistry )
{
	static GPIPipelineStateDesc pipelineDesc{};
	static IGPIPipelineRef pipeline = nullptr;
	if( !pipeline )
	{
		pipelineDesc.id = 1;
		pipelineDesc.pipelineType = PipelineType_Graphics;
		pipelineDesc.bBindDepth = false;

		pipelineDesc.rtvFormats = { EGPIResourceFormat::B8G8R8A8_SRGB };

		pipelineDesc.inputDesc = {
			{ "POSITION", EGPIResourceFormat::R32G32B32_Float, GPIInputClass_PerVertex, 0 },
			{ "TEXCOORD", EGPIResourceFormat::R32G32_Float, GPIInputClass_PerVertex, 2 }
		};

		pipelineDesc.vertexShader.hash = 2;
		pipelineDesc.vertexShader.type = ShaderType_VertexShader;
		pipelineDesc.vertexShader.file = "Engine/Shader/Lighting.hlsl";
		pipelineDesc.vertexShader.entry = "VS_DirectionalLight";

		pipelineDesc.pixelShader.hash = 3;
		pipelineDesc.pixelShader.type = ShaderType_PixelShader;
		pipelineDesc.pixelShader.file = "Engine/Shader/Lighting.hlsl";
		pipelineDesc.pixelShader.entry = "PS_DirectionalLight";

		pipelineDesc.numCBVs = 1;
		pipelineDesc.numTextures = { 4 };

		pipeline = AtomicEngine::GetGPI()->CreatePipelineState( pipelineDesc );
	}

	if( !_sceneLightResource )
	{
		const IVec2 windowSize = AtomicEngine::GetGPI()->GetWindowSize();
		const GPIResourceDesc sceneLightDesc = GPIUtil::GetRenderTargetResourceDesc( L"SceneLight", windowSize );
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

		_sceneLightUAV = AtomicEngine::GetGPI()->CreateUnorderedAccessView( *_sceneLightResource, uavDesc, true );
	}

	{// @TODO: move to somewhere makes sense
		AtomicEngine::GetGPI()->BindRenderTargetView( *pipeline, *_sceneLightRTV, 0 );

		AtomicEngine::GetGPI()->BindConstantBufferView( *pipeline, *_viewCBV, 0 );

		AtomicEngine::GetGPI()->BindTextureViewTable( *pipeline, *_gBufferTextureViewTable, 0 );
	}

	const StaticMeshRef& staticMesh = StaticMeshCache::FindStaticMesh( "quad" );

	AtomicEngine::GetGPI()->ClearRenderTarget( *_sceneLightUAV );

	AtomicEngine::GetGPI()->SetPipelineState( pipelineDesc );

	AtomicEngine::GetGPI()->Render( staticMesh->pipelineInput );

	AtomicEngine::GetGPI()->ExecuteCommandList();
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

	static GPIPipelineStateDesc pipelineDesc{};
	static IGPIPipelineRef pipeline = nullptr;
	if( !pipeline )
	{
		pipelineDesc.id = 2;
		pipelineDesc.pipelineType = PipelineType_Graphics;
		pipelineDesc.bBindDepth = false;
		pipelineDesc.bEnableBlend = true;

		pipelineDesc.rtvFormats = { EGPIResourceFormat::B8G8R8A8_SRGB };

		pipelineDesc.inputDesc = {
			{ "POSITION", EGPIResourceFormat::R32G32B32_Float, GPIInputClass_PerVertex, 0 },
			{ "TEXCOORD", EGPIResourceFormat::R32G32_Float, GPIInputClass_PerVertex, 2 }
		};

		pipelineDesc.vertexShader.hash = 4;
		pipelineDesc.vertexShader.type = ShaderType_VertexShader;
		pipelineDesc.vertexShader.file = "Engine/Shader/Lighting.hlsl";
		pipelineDesc.vertexShader.entry = "VS_PointLight";

		pipelineDesc.pixelShader.hash = 5;
		pipelineDesc.pixelShader.type = ShaderType_PixelShader;
		pipelineDesc.pixelShader.file = "Engine/Shader/Lighting.hlsl";
		pipelineDesc.pixelShader.entry = "PS_PointLight";

		pipelineDesc.numCBVs = 2;
		pipelineDesc.numTextures = { 4 };

		pipeline = AtomicEngine::GetGPI()->CreatePipelineState( pipelineDesc );
	}

	{// @TODO: move to somewhere makes sense
		AtomicEngine::GetGPI()->BindRenderTargetView( *pipeline, *_sceneLightRTV, 0 );

		AtomicEngine::GetGPI()->BindTextureViewTable( *pipeline, *_gBufferTextureViewTable, 0 );
	}

	__declspec( align( 256 ) )
		struct LightConstantBuffer
	{
		Mat4x4 matModel;
		Vec3 origin;
		float intensity;
	};

	static IGPIResourceRef lightCBResource = nullptr;
	static IGPIConstantBufferViewRef lightCBV;
	if( !lightCBResource )
	{
		const GPIResourceDesc cbDesc = GPIUtil::GetConstantBufferResourceDesc( L"LightConstant", sizeof( LightConstantBuffer ) );
		lightCBResource = AtomicEngine::GetGPI()->CreateResource( cbDesc );

		GPIConstantBufferViewDesc cbvDesc{};
		cbvDesc.sizeInBytes = sizeof( LightConstantBuffer );

		lightCBV = AtomicEngine::GetGPI()->CreateConstantBufferView( *lightCBResource, cbvDesc );
	}

	{// @TODO: move to somewhere makes sense
		AtomicEngine::GetGPI()->BindConstantBufferView( *pipeline, *_viewCBV, 0 );
		AtomicEngine::GetGPI()->BindConstantBufferView( *pipeline, *lightCBV, 1 );
	}

	const StaticMeshRef& staticMesh = StaticMeshCache::FindStaticMesh( "sphere" );

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

		AtomicEngine::GetGPI()->UpdateResourceData( *lightCBResource, &lightBuffer, sizeof( lightBuffer ) );

		AtomicEngine::GetGPI()->SetPipelineState( pipelineDesc );

		AtomicEngine::GetGPI()->Render( staticMesh->pipelineInput );

		AtomicEngine::GetGPI()->ExecuteCommandList();
	}
}

void RenderSystem::LightCombine( std::array<std::unique_ptr<IComponentRegistry>, NUM_COMPONENT_MAX>& componentRegistry )
{
	static GPIPipelineStateDesc pipelineDesc{};
	static IGPIPipelineRef pipeline = nullptr;
	if( !pipeline )
	{
		pipelineDesc.id = 3;
		pipelineDesc.pipelineType = PipelineType_Graphics;
		pipelineDesc.bBindDepth = false;

		pipelineDesc.rtvFormats = { EGPIResourceFormat::B8G8R8A8_SRGB };

		pipelineDesc.inputDesc = {
			{ "POSITION", EGPIResourceFormat::R32G32B32_Float, GPIInputClass_PerVertex, 0 },
			{ "TEXCOORD", EGPIResourceFormat::R32G32_Float, GPIInputClass_PerVertex, 2 }
		};

		pipelineDesc.vertexShader.hash = 6;
		pipelineDesc.vertexShader.type = ShaderType_VertexShader;
		pipelineDesc.vertexShader.file = "Engine/Shader/Lighting.hlsl";
		pipelineDesc.vertexShader.entry = "VS_LightCombine";

		pipelineDesc.pixelShader.hash = 7;
		pipelineDesc.pixelShader.type = ShaderType_PixelShader;
		pipelineDesc.pixelShader.file = "Engine/Shader/Lighting.hlsl";
		pipelineDesc.pixelShader.entry = "PS_LightCombine";

		pipelineDesc.numTextures = { 4, 1 };

		pipeline = AtomicEngine::GetGPI()->CreatePipelineState( pipelineDesc );
	}

	{// @TODO: move to somewhere makes sense
		uint32 swapChainIndex = AtomicEngine::GetGPI()->GetSwapChainCurrentIndex();
		IGPIRenderTargetViewRef& swapChainRTV = _swapChainRTV[ swapChainIndex ];
		AtomicEngine::GetGPI()->BindRenderTargetView( *pipeline, *_swapChainRTV[ swapChainIndex ], 0 );

		AtomicEngine::GetGPI()->BindTextureViewTable( *pipeline, *_gBufferTextureViewTable, 0 );
		AtomicEngine::GetGPI()->BindTextureViewTable( *pipeline, *_sceneLightTextureViewTable, 1 );
	}

	const StaticMeshRef& staticMesh = StaticMeshCache::FindStaticMesh( "quad" );

	AtomicEngine::GetGPI()->SetPipelineState( pipelineDesc );

	AtomicEngine::GetGPI()->Render( staticMesh->pipelineInput );

	AtomicEngine::GetGPI()->ExecuteCommandList();
}
