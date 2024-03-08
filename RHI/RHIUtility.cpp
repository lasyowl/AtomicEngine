#include "RHIUtility.h"
#include <Core/IntVector.h>

#include <dxgi.h>

namespace RHIUtil
{
RHIResourceDesc GetConstantBufferResourceDesc( const std::wstring& name, const uint32 byteSize )
{
	RHIResourceDesc desc{};
	desc.name = name;
	desc.dimension = ERHIResourceDimension::Buffer;
	desc.format = ERHIResourceFormat::Unknown;
	desc.width = byteSize;
	desc.height = 1;
	desc.depth = 1;
	desc.numMips = 1;
	desc.flags = RHIResourceFlag_None;

	return desc;
}

RHIResourceDesc GetRenderTargetResourceDesc( const std::wstring& name, const IVec2& size )
{
	RHIResourceDesc desc{};
	desc.name = name;
	desc.dimension = ERHIResourceDimension::Texture2D;
	desc.format = ERHIResourceFormat::B8G8R8A8;
	desc.width = size.x;
	desc.height = size.y;
	desc.depth = 1;
	desc.numMips = 1;
	desc.flags = RHIResourceFlag_AllowRenderTarget | RHIResourceFlag_AllowUnorderedAccess;
	desc.initialState = RHIResourceState_RenderTarget;
	desc.clearValue.type = ERHIResourceClearValueType::Color;
	desc.clearValue.color = Vec4( 0.0f, 0.0f, 0.0f, 0.0f );

	return desc;
}

RHIResourceDesc GetDepthStencilResourceDesc( const std::wstring& name, const IVec2& size )
{
	RHIResourceDesc desc{};
	desc.name = L"DepthStencil";
	desc.dimension = ERHIResourceDimension::Texture2D;
	desc.format = ERHIResourceFormat::D32_Float;
	desc.width = size.x;
	desc.height = size.y;
	desc.depth = 1;
	desc.numMips = 1;
	desc.flags = RHIResourceFlag_AllowDepthStencil;
	desc.initialState = RHIResourceState_DepthWrite;
	desc.clearValue.type = ERHIResourceClearValueType::DepthStencil;
	desc.clearValue.depthStencil = Vec2( 1.0f, 0.0f );

	return desc;
}

RHIResourceDesc GetVertexResourceDesc( const std::wstring& name, const uint32 byteSize )
{
	RHIResourceDesc desc{};
	desc.name = name;
	desc.dimension = ERHIResourceDimension::Buffer;
	desc.format = ERHIResourceFormat::Unknown;
	desc.width = byteSize;
	desc.height = 1;
	desc.depth = 1;
	desc.numMips = 1;
	desc.flags = RHIResourceFlag_None;

	return desc;
}

RHIResourceDesc GetRayTraceResourceDesc( const std::wstring& name, const uint32 byteSize )
{
	RHIResourceDesc desc{};
	desc.name = name;
	desc.dimension = ERHIResourceDimension::Buffer;
	desc.format = ERHIResourceFormat::Unknown;
	desc.width = byteSize;
	desc.height = 1;
	desc.depth = 1;
	desc.numMips = 1;
	desc.flags = RHIResourceFlag_None;

	return desc;
}
}