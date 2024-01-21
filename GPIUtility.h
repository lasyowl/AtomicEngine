#pragma once

#include "GPIResource.h"
#include "IntVector.h"

namespace GPIUtil
{
GPIResourceDesc GetConstantBufferResourceDesc( const std::wstring& name, const uint32 byteSize );
GPIResourceDesc GetRenderTargetResourceDesc( const std::wstring& name, const IVec2& size );
GPIResourceDesc GetDepthStencilResourceDesc( const std::wstring& name, const IVec2& size );
}