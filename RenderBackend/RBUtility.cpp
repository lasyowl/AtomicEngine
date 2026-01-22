#include "RBUtility.h"
#include <Core/IntVector.h>

#include <dxgi.h>

namespace RBUtil
{
    RBResourceDesc GetConstantBufferResourceDesc( const std::string& name, const uint32 byteSize )
    {
        RBResourceDesc desc{};
        desc.name = name;
        desc.dimension = ERBResourceDimension::Buffer;
        desc.format = ERBResourceFormat::Unknown;
        desc.width = byteSize;
        desc.height = 1;
        desc.depth = 1;
        desc.numMips = 1;
        desc.flags = RBResourceFlag_None;

        return desc;
    }

    RBResourceDesc GetRenderTargetResourceDesc( const std::string& name, const IVec2& size )
    {
        RBResourceDesc desc{};
        desc.name = name;
        desc.dimension = ERBResourceDimension::Texture2D;
        desc.format = ERBResourceFormat::B8G8R8A8;
        desc.width = size.x;
        desc.height = size.y;
        desc.depth = 1;
        desc.numMips = 1;
        desc.flags = RBResourceFlag_AllowRenderTarget | RBResourceFlag_AllowUnorderedAccess;
        desc.initialState = RBResourceState_RenderTarget;
        desc.clearValue.type = ERBResourceClearValueType::Color;
        desc.clearValue.color = Vec4( 0.0f, 0.0f, 0.0f, 0.0f );

        return desc;
    }

    RBResourceDesc GetDepthStencilResourceDesc( const IVec2& size )
    {
        RBResourceDesc desc{};
        desc.name = "DepthStencil";
        desc.dimension = ERBResourceDimension::Texture2D;
        desc.format = ERBResourceFormat::D32_Float;
        desc.width = size.x;
        desc.height = size.y;
        desc.depth = 1;
        desc.numMips = 1;
        desc.flags = RBResourceFlag_AllowDepthStencil;
        desc.initialState = RBResourceState_DepthWrite;
        desc.clearValue.type = ERBResourceClearValueType::DepthStencil;
        desc.clearValue.depthStencil = Vec2( 1.0f, 0.0f );

        return desc;
    }

    RBResourceDesc GetVertexResourceDesc( const std::string& name, const uint32 byteSize )
    {
        RBResourceDesc desc{};
        desc.name = name;
        desc.dimension = ERBResourceDimension::Buffer;
        desc.format = ERBResourceFormat::Unknown;
        desc.width = byteSize;
        desc.height = 1;
        desc.depth = 1;
        desc.numMips = 1;
        desc.flags = RBResourceFlag_None;
        desc.initialState = RBResourceState_VertexConstantBuffer;

        return desc;
    }

    RBResourceDesc GetIndexResourceDesc( const std::string& name, const uint32 byteSize )
    {
        RBResourceDesc desc{};
        desc.name = name;
        desc.dimension = ERBResourceDimension::Buffer;
        desc.format = ERBResourceFormat::Unknown;
        desc.width = byteSize;
        desc.height = 1;
        desc.depth = 1;
        desc.numMips = 1;
        desc.flags = RBResourceFlag_None;
        desc.initialState = RBResourceState_IndexBuffer;

        return desc;
    }

    RBResourceDesc GetRayTraceResourceDesc( const std::string& name, const uint32 byteSize )
    {
        RBResourceDesc desc{};
        desc.name = name;
        desc.dimension = ERBResourceDimension::Buffer;
        desc.format = ERBResourceFormat::Unknown;
        desc.width = byteSize;
        desc.height = 1;
        desc.depth = 1;
        desc.numMips = 1;
        desc.flags = RBResourceFlag_None;

        return desc;
    }
}