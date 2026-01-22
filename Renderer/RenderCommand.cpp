#include "RenderCommand.h"

std::vector<RenderCommand> commandQueues[ 2 ];

std::atomic<uint64> engineThreadFrame = 0;
std::atomic<uint64> renderThreadFrame = 0;

std::mutex mutex;

namespace RenderUtil
{
    void EnqueueRenderCommand( RenderCommand&& renderCommand )
    {
        const int32 writeIndex = engineThreadFrame % 2;
        commandQueues[ writeIndex ].emplace_back( renderCommand );
    }

    void PublishRenderCommands()
    {
        // @TODO: Assert engine thread only

        while ( engineThreadFrame != renderThreadFrame.load() )
        {
            // Wait for the render thread to catch up
            Sleep( 0 );
        }

        engineThreadFrame.fetch_add( 1 );

        {
            std::lock_guard lock( mutex );
            // Commands are now ready for the render thread
        }

        //std::lock_guard lock( mutex );
        //std::wstring log = std::wstring( L"EngineThread: " ) + std::to_wstring( engineThreadFrame.load() ) + std::wstring( L"\n" );
        //OutputDebugStringW( log.c_str() );
    }

    void FlushRenderCommand( IRenderBackend& rb )
    {
        // @TODO: Assert render thread only

        while ( engineThreadFrame.load() == renderThreadFrame )
        {
            // Wait for the engine thread to publish new commands
            Sleep( 0 );
        }

        std::vector<RenderCommand> commandsToExecute;
        {
            std::lock_guard lock( mutex );

            const int32 readIndex = renderThreadFrame % 2;
            commandsToExecute = std::move( commandQueues[ readIndex ] );
        }

        renderThreadFrame.fetch_add( 1 );

        for ( auto& command : commandsToExecute )
        {
            command( rb );
        }

        //std::lock_guard lock( mutex );
        //std::wstring log = std::wstring( L"RenderThread: " ) + std::to_wstring( renderThreadFrame.load() ) + std::wstring( L"\n" );
        //OutputDebugStringW( log.c_str() );
    }
}
