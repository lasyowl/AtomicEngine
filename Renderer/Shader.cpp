#include "Shader.h"

#include <RenderBackend/RB.h>

void ShaderBase::CreateRenderResources( IRenderBackend& rb )
{
    _resource = rb.CreateShader( _desc );
}
