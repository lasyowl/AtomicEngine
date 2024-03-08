#pragma once

#include <Core/IntVector.h>
#include "RHIResource.h"

namespace RHIUtil
{
RHIResourceDesc GetConstantBufferResourceDesc( const std::wstring& name, const uint32 byteSize );
RHIResourceDesc GetRenderTargetResourceDesc( const std::wstring& name, const IVec2& size );
RHIResourceDesc GetDepthStencilResourceDesc( const std::wstring& name, const IVec2& size );
RHIResourceDesc GetVertexResourceDesc( const std::wstring& name, const uint32 byteSize );
RHIResourceDesc GetRayTraceResourceDesc( const std::wstring& name, const uint32 byteSize );
}