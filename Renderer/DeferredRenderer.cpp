#include "DeferredRenderer.h"

#include <Core/Matrix.h>
#include <RenderBackend/RB_DX12.h>
#include <Renderer/View.h>
#include <Renderer/RenderCommand.h>
#include <Renderer/Scene.h>
#include <Renderer/Mesh.h>
#include <Renderer/Material.h>
#include <Renderer/Texture.h>
#include <Renderer/Shader.h>

DeferredRenderer::DeferredRenderer( HWND hWnd, Scene& scene )
    : _scene( scene )
    , _viewportSize( 1280, 720 )
    , _rb( nullptr )
{
    /* @TODO : Move to somewhere makes sense */
    HMODULE hm = LoadLibrary( L"C:\\Program Files\\Microsoft PIX\\2312.08\\WinPixGpuCapturer.dll" );

    /* @TODO : Run graphics API by compatibility */
    IVec2 windowSize = IVec2( 1920, 1080 );
    _rb = std::make_unique<RB_DX12>( hWnd, windowSize );
    _rb->Startup();
    _rb->SetWindowSize( windowSize );
}

void DeferredRenderer::PrepareDefaultResources()
{
    // @TODO: Replace begin/end frame with proper render backend flow
    _rb->BeginFrame();

    MeshCache::BuildSamples( *_rb );

    Texture::White.CreateRenderResources( *_rb );

    _gBuffers.gBufferA.CreateRenderResources( *_rb, 1920, 1080 );
    _gBuffers.gBufferB.CreateRenderResources( *_rb, 1920, 1080 );
    _gBuffers.rtvs = { _gBuffers.gBufferA.GetView(), _gBuffers.gBufferB.GetView() };

    _rb->EndFrame();
}

void DeferredRenderer::BeginFrame()
{
    _rb->BeginFrame();

    // @TODO: Remove test case
    static bool bFirstFrame = true;
    if ( bFirstFrame )
    {
        MeshCache::BuildSamples( *_rb );

        Texture::White.CreateRenderResources( *_rb );

        bFirstFrame = false;
    }

    RenderUtil::FlushRenderCommand( *_rb );
}

void DeferredRenderer::RenderFrame()
{
    RenderGeometryPass();
}

void DeferredRenderer::EndFrame()
{
    _rb->EndFrame();
}

void DeferredRenderer::RenderGeometryPass()
{
    static GeometryVS vertexShader;
    static GeometryPS pixelShader;
    if ( !vertexShader._resource )
    {
        vertexShader.CreateRenderResources( *_rb );
        pixelShader.CreateRenderResources( *_rb );
    }

    static RBPipelineStateDesc pipelineDesc =
    {
        .pipelineType = PipelineType_Graphics,
        .enableDepth = true,
        .enableBlend = true,
        .inputDesc =
        {
            { "POSITION", ERBResourceFormat::R32G32B32_Float, RBInputClass_PerVertex, 0, 0 },
            { "NORMAL", ERBResourceFormat::R32G32B32_Float, RBInputClass_PerVertex, 0, sizeof( Vec3 ) },
            { "TEXCOORD", ERBResourceFormat::R32G32_Float, RBInputClass_PerVertex, 0, sizeof( Vec3 ) + sizeof( Vec3 ) },
            { "TANGENT", ERBResourceFormat::R32G32B32_Float, RBInputClass_PerVertex, 0, sizeof( Vec3 ) + sizeof( Vec3 ) + sizeof( Vec2 ) }
        },
        .vertexShader = vertexShader._resource,
        .pixelShader = pixelShader._resource,
        .vertexShaderDesc = &vertexShader._desc,
        .pixelShaderDesc = &pixelShader._desc,
        .pixelShaderParameterOffset = vertexShader.GetParameterCount(),
        .rtvFormats = { ERBResourceFormat::B8G8R8A8, ERBResourceFormat::B8G8R8A8 }
    };

    static PipelineStateObject pso;
    if ( !pso.GetResource() )
    {
        pso.CreateRenderResources( *_rb, pipelineDesc );
    }

    ViewEntity* viewEntity = _scene.GetViewEntities().front();
    viewEntity->UpdateViewProjectionMatrix( *_rb );

    _rb->SetGraphicsPipelineState( *pso.GetResource() );

    _rb->BindConstantBufferView( *viewEntity->GetConstantBufferView(), GeometryVS::RPI_PerFrame );

    _rb->BindRenderTargetViews( _gBuffers.rtvs, _rb->GetSwapChainDepthStencilView() );
    _rb->ClearRenderTarget( *_gBuffers.gBufferA.GetResource(), *_gBuffers.gBufferA.GetView() );
    _rb->ClearRenderTarget( *_gBuffers.gBufferB.GetResource(), *_gBuffers.gBufferB.GetView() );

    for ( MeshEntity* entity : _scene.GetMeshEntities() )
    {
        const MeshData& meshData = entity->GetMeshData();

        _rb->BindConstantBufferView( *entity->GetConstantBufferView(), GeometryVS::RPI_PerDraw );

        Material* material = entity->GetMaterial();
        if ( material->textureViewTable )
        {
            const uint32 pixelShaderRootParamIndex = vertexShader.GetParameterCount() + GeometryPS::RPI_TexBegin;
            _rb->BindTextureViewTable( *material->textureViewTable, pixelShaderRootParamIndex );
        }

        _rb->SetInputAssembly( *entity->GetVertexBufferView(), *entity->GetIndexBufferView() );
        _rb->DrawIndexedInstanced( meshData.GetNumIndices(), 1, 0, 0, 0 );
    }

    IRBResource* swapChainResource = _rb->GetSwapChainColorResource();
    _rb->CopyTextureRegion( *swapChainResource, *_gBuffers.gBufferA.GetResource(), 1920, 1080 );

    _rb->ExecuteCommandList();
}

void DeferredRenderer::RenderLightingPass()
{
    static FullScreenQuadVS vertexShader;
    static LightingPS pixelShader;
    if ( !vertexShader._resource )
    {
        vertexShader.CreateRenderResources( *_rb );
        pixelShader.CreateRenderResources( *_rb );
    }

    static RBPipelineStateDesc pipelineDesc =
    {
        .pipelineType = PipelineType_Graphics,
        .enableDepth = false,
        .enableBlend = false,
        .inputDesc = { { "POSITION", ERBResourceFormat::R32G32B32_Float, RBInputClass_PerVertex, 0, 0 } },
        .vertexShader = vertexShader._resource,
        .pixelShader = pixelShader._resource,
        .vertexShaderDesc = &vertexShader._desc,
        .pixelShaderDesc = &pixelShader._desc,
        .pixelShaderParameterOffset = vertexShader.GetParameterCount(),
        .rtvFormats = { ERBResourceFormat::B8G8R8A8 }
    };

    static PipelineStateObject pso;
    if ( !pso.GetResource() )
    {
        pso.CreateRenderResources( *_rb, pipelineDesc );
    }

    _rb->SetGraphicsPipelineState( *pso.GetResource() );

    //ViewEntity* viewEntity = _scene.GetViewEntities().front();
    //_rb->BindConstantBufferView( *viewEntity->GetConstantBufferView(), GeometryVS::RPI_PerFrame );

    std::vector<const IRBRenderTargetView*> swapChainRtvs = { _rb->GetSwapChainRenderTargetView() };
    _rb->BindRenderTargetViews( swapChainRtvs, nullptr );

    for ( MeshEntity* entity : _scene.GetMeshEntities() )
    {
        const MeshData& meshData = entity->GetMeshData();

        _rb->BindConstantBufferView( *entity->GetConstantBufferView(), GeometryVS::RPI_PerDraw );

        Material* material = entity->GetMaterial();
        if ( material->textureViewTable )
        {
            const uint32 pixelShaderRootParamIndex = vertexShader.GetParameterCount() + GeometryPS::RPI_TexBegin;
            _rb->BindTextureViewTable( *material->textureViewTable, pixelShaderRootParamIndex );
        }

        _rb->SetInputAssembly( *entity->GetVertexBufferView(), *entity->GetIndexBufferView() );
        _rb->DrawIndexedInstanced( meshData.GetNumIndices(), 1, 0, 0, 0 );
    }

    IRBResource* swapChainResource = _rb->GetSwapChainColorResource();
    _rb->CopyTextureRegion( *swapChainResource, *_gBuffers.gBufferA.GetResource(), 1920, 1080 );

    _rb->ExecuteCommandList();
}
