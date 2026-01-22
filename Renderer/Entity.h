#pragma once

#include <Renderer/RenderResource.h>

class IRenderBackend;
struct MeshData;

class IEntity
{
public:
    virtual ~IEntity() = default;

    virtual void CreateRenderResources( IRenderBackend& rb ) abstract;
};
