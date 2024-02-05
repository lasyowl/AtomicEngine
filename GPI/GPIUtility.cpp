#include "GPIUtility.h"
#include <Core/IntVector.h>

#include <dxgi.h>

namespace GPIUtil
{
GPIResourceDesc GetConstantBufferResourceDesc( const std::wstring& name, const uint32 byteSize )
{
	GPIResourceDesc desc{};
	desc.name = name;
	desc.dimension = EGPIResourceDimension::Buffer;
	desc.format = EGPIResourceFormat::Unknown;
	desc.width = byteSize;
	desc.height = 1;
	desc.depth = 1;
	desc.numMips = 1;
	desc.flags = GPIResourceFlag_None;

	return desc;
}

GPIResourceDesc GetRenderTargetResourceDesc( const std::wstring& name, const IVec2& size )
{
	GPIResourceDesc desc{};
	desc.name = name;
	desc.dimension = EGPIResourceDimension::Texture2D;
	desc.format = EGPIResourceFormat::B8G8R8A8;
	desc.width = size.x;
	desc.height = size.y;
	desc.depth = 1;
	desc.numMips = 1;
	desc.flags = GPIResourceFlag_AllowRenderTarget | GPIResourceFlag_AllowUnorderedAccess;
	desc.initialState = GPIResourceState_RenderTarget;
	desc.clearValue.type = EGPIResourceClearValueType::Color;
	desc.clearValue.color = Vec4( 0.0f, 0.0f, 0.0f, 0.0f );

	return desc;
}

GPIResourceDesc GetDepthStencilResourceDesc( const std::wstring& name, const IVec2& size )
{
	GPIResourceDesc desc{};
	desc.name = L"DepthStencil";
	desc.dimension = EGPIResourceDimension::Texture2D;
	desc.format = EGPIResourceFormat::D32_Float;
	desc.width = size.x;
	desc.height = size.y;
	desc.depth = 1;
	desc.numMips = 1;
	desc.flags = GPIResourceFlag_AllowDepthStencil;
	desc.initialState = GPIResourceState_DepthWrite;
	desc.clearValue.type = EGPIResourceClearValueType::DepthStencil;
	desc.clearValue.depthStencil = Vec2( 1.0f, 0.0f );

	return desc;
}

GPIResourceDesc GetVertexResourceDesc( const std::wstring& name, const uint32 byteSize )
{
	GPIResourceDesc desc{};
	desc.name = name;
	desc.dimension = EGPIResourceDimension::Buffer;
	desc.format = EGPIResourceFormat::Unknown;
	desc.width = byteSize;
	desc.height = 1;
	desc.depth = 1;
	desc.numMips = 1;
	desc.flags = GPIResourceFlag_None;

	return desc;
}

GPIResourceDesc GetRayTraceResourceDesc( const std::wstring& name, const uint32 byteSize )
{
	GPIResourceDesc desc{};
	desc.name = name;
	desc.dimension = EGPIResourceDimension::Buffer;
	desc.format = EGPIResourceFormat::Unknown;
	desc.width = byteSize;
	desc.height = 1;
	desc.depth = 1;
	desc.numMips = 1;
	desc.flags = GPIResourceFlag_None;

	return desc;
}
}