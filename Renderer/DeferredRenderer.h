#pragma once

#include "Renderer.h"

#include <Core/IntVector.h>
#include <Renderer/RenderResource.h>

class Scene;
class IRenderBackend;

struct GBuffers
{
    RenderTargetBuffer gBufferA;
    RenderTargetBuffer gBufferB;
    std::vector<const IRBRenderTargetView*> rtvs;
};

class DeferredRenderer : public IRenderer
{
public:
    DeferredRenderer( HWND hWnd, Scene& scene );
    virtual ~DeferredRenderer() = default;

    virtual void PrepareDefaultResources() override;

    virtual void BeginFrame() override;
    virtual void RenderFrame() override;
    virtual void EndFrame() override;

private:
    void RenderShadowPass();
    void RenderGeometryPass();
    void RenderLightingPass();
    void RenderPostProcessPass();
    void RenderUIPass();

private:
    Scene& _scene;

    GBuffers _gBuffers;

    std::unique_ptr<IRenderBackend> _rb;

    // @TODO: Wrap with view related class
    IVec2 _viewportSize;
};
