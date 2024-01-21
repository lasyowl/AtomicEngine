#pragma once

#include "GPIResource.h"

namespace GPIUtil
{
GPIResourceDesc GetConstantBufferResourceDesc( const std::wstring& name, const uint32 byteSize );
GPIResourceDesc GetRenderTargetResourceDesc( const std::wstring& name, const uint32 width, const uint32 height );
GPIResourceDesc GetDepthStencilResourceDesc( const std::wstring& name, const uint32 width, const uint32 height );
}