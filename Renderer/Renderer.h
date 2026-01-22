#pragma once

class IRenderer
{
public:
    IRenderer() = default;
    virtual ~IRenderer() = default;

    virtual void PrepareDefaultResources() = 0;

    virtual void BeginFrame() = 0;
    virtual void RenderFrame() = 0;
    virtual void EndFrame() = 0;
};