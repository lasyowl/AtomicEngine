#pragma once

#include <RenderBackend/RB.h>
#include <RenderBackend/RBResource.h>

class VertexBuffer
{
public:
    void CreateRenderResources( IRenderBackend& rb, const void* data, uint32 sizeInBytes, uint32 stride );

    IRBResource* GetResource() const { return _resource.get(); }
    IRBVertexBufferView* GetView() const { return _view.get(); }

private:
    IRBResourcePtr _resource;
    IRBVertexBufferViewPtr _view;
};

class IndexBuffer
{
public:
    void CreateRenderResources( IRenderBackend& rb, const void* data, uint32 sizeInBytes );

    IRBResource* GetResource() const { return _resource.get(); }
    IRBIndexBufferView* GetView() const { return _view.get(); }

private:
    IRBResourcePtr _resource;
    IRBIndexBufferViewPtr _view;
};

class ConstantBuffer
{
public:
    void CreateRenderResources( IRenderBackend& rb, const void* data, uint32 sizeInBytes );

    IRBResource* GetResource() const { return _resource.get(); }
    IRBConstantBufferView* GetView() const { return _view.get(); }

private:
    IRBResourcePtr _resource;
    IRBConstantBufferViewPtr _view;
};

class TextureBuffer
{
public:
    void CreateRenderResources( IRenderBackend& rb, const void* data, uint32 width, uint32 height );

    IRBResource* GetResource() const { return _resource.get(); }
    IRBShaderResourceView* GetView() const { return _view.get(); }

private:
    IRBResourcePtr _resource;
    IRBShaderResourceViewPtr _view;
};

class RenderTargetBuffer
{
public:
    void CreateRenderResources( IRenderBackend& rb, uint32 width, uint32 height );

    IRBResource* GetResource() const { return _resource.get(); }
    IRBRenderTargetView* GetView() const { return _view.get(); }

private:
    IRBResourcePtr _resource;
    IRBRenderTargetViewPtr _view;
};

class PipelineStateObject
{
public:
    void CreateRenderResources( IRenderBackend& rb, const RBPipelineStateDesc& desc );
    IRBPipelineRef GetResource() const { return _resource; }

private:
    IRBPipelineRef _resource;
};
