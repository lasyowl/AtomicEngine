#pragma once

#include <Core/IntVector.h>
#include "RBResource.h"

namespace RBUtil
{
    RBResourceDesc GetConstantBufferResourceDesc( const std::string& name, const uint32 byteSize );
    RBResourceDesc GetRenderTargetResourceDesc( const std::string& name, const IVec2& size );
    RBResourceDesc GetDepthStencilResourceDesc( const IVec2& size );
    RBResourceDesc GetVertexResourceDesc( const std::string& name, const uint32 byteSize );
    RBResourceDesc GetIndexResourceDesc( const std::string& name, const uint32 byteSize );
    RBResourceDesc GetRayTraceResourceDesc( const std::string& name, const uint32 byteSize );
}
