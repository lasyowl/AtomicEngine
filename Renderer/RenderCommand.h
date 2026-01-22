#pragma once

class IRenderBackend;

using RenderCommand = std::function<void( IRenderBackend& rb )>;

namespace RenderUtil
{
    void EnqueueRenderCommand( RenderCommand&& renderCommand );
    void PublishRenderCommands();

    void FlushRenderCommand( IRenderBackend& rb );
}
