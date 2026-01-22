#include "RenderResource.h"

#include <RenderBackend/RB.h>
#include <RenderBackend/RBResource.h>
#include <RenderBackend/RBUtility.h>

void VertexBuffer::CreateRenderResources( IRenderBackend& rb, const void* data, uint32 sizeInBytes, uint32 stride )
{
    RBResourceDesc resourceDesc = RBUtil::GetVertexResourceDesc( "MeshVertexBuffer", sizeInBytes );

    _resource = rb.CreateResource( resourceDesc, data, sizeInBytes );
    _view = rb.CreateVertexBufferView( *_resource, sizeInBytes, stride );
}

void IndexBuffer::CreateRenderResources( IRenderBackend& rb, const void* data, uint32 sizeInBytes )
{
    RBResourceDesc resourceDesc = RBUtil::GetIndexResourceDesc( "MeshIndexBuffer", sizeInBytes );

    _resource = rb.CreateResource( resourceDesc, data, sizeInBytes );
    _view = rb.CreateIndexBufferView( *_resource, sizeInBytes );
}

void ConstantBuffer::CreateRenderResources( IRenderBackend& rb, const void* data, uint32 sizeInBytes )
{
    RBResourceDesc resourceDesc = RBUtil::GetConstantBufferResourceDesc( "ConstantBuffer", sizeInBytes );

    RBConstantBufferViewDesc viewDesc{};
    viewDesc.bStatic = false;
    viewDesc.sizeInBytes = sizeInBytes;

    _resource = rb.CreateResource( resourceDesc, data, sizeInBytes );
    _view = rb.CreateConstantBufferView( *_resource, viewDesc );

    rb.UpdateResourceData( *_resource, data, sizeInBytes );
}

void TextureBuffer::CreateRenderResources( IRenderBackend& rb, const void* data, uint32 width, uint32 height )
{
    RBResourceDesc resourceDesc{};
    resourceDesc.dimension = ERBResourceDimension::Texture2D;
    resourceDesc.format = ERBResourceFormat::B8G8R8A8;
    resourceDesc.width = width;
    resourceDesc.height = height;
    resourceDesc.depth = 1;
    resourceDesc.numMips = 1;
    resourceDesc.clearValue.type = ERBResourceClearValueType::None;
    resourceDesc.flags = RBResourceFlag_None;
    resourceDesc.initialState = RBResourceState_PixelShaderResource;

    RBShaderResourceViewDesc viewDesc{};
    viewDesc.bStatic = true;
    viewDesc.dimension = ERBResourceDimension::Texture2D;
    viewDesc.format = ERBResourceFormat::B8G8R8A8;

    _resource = rb.CreateResource( resourceDesc );
    rb.UpdateTextureData( *_resource, data, width, height );

    _view = rb.CreateShaderResourceView( *_resource, viewDesc );
}

void RenderTargetBuffer::CreateRenderResources( IRenderBackend& rb, uint32 width, uint32 height )
{
    RBResourceDesc resourceDesc{};
    resourceDesc.dimension = ERBResourceDimension::Texture2D;
    resourceDesc.format = ERBResourceFormat::B8G8R8A8;
    resourceDesc.width = 1920;
    resourceDesc.height = 1080;
    resourceDesc.depth = 1;
    resourceDesc.numMips = 1;
    resourceDesc.clearValue.type = ERBResourceClearValueType::Color;
    resourceDesc.clearValue.color = Vec4( 0.0f, 0.0f, 0.0f, 0.0f );
    resourceDesc.flags = RBResourceFlag_AllowRenderTarget | RBResourceFlag_AllowUnorderedAccess;
    resourceDesc.initialState = RBResourceState_RenderTarget;

    RBRenderTargetViewDesc viewDesc{};
    viewDesc.bStatic = false;
    viewDesc.format = ERBResourceFormat::B8G8R8A8;
    viewDesc.dimension = ERBResourceDimension::Texture2D;

    _resource = rb.CreateResource( resourceDesc );
    _view = rb.CreateRenderTargetView( *_resource, viewDesc );
}

void PipelineStateObject::CreateRenderResources( IRenderBackend& rb, const RBPipelineStateDesc& desc )
{
    _resource = rb.CreateGraphicsPipelineState( desc );
}
