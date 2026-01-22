#include <Engine/FeatherEntry.h>
#include <RenderBackend/RB_DX12.h>
#include <Core/IntVector.h>
#include <Engine/TestScene.h>
#include <Engine/World.h>
#include <Engine/InputSystem.h>
#include <Renderer/DeferredRenderer.h>
#include <Renderer/Scene.h>

namespace
{
    LRESULT CALLBACK WindowMessageProcessor( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
    {
        switch ( msg )
        {
            case WM_DESTROY:
            {
                PostQuitMessage( 0 );
                return 0;
            }
        }
        return DefWindowProc( hWnd, msg, wParam, lParam );
    }

    HWND CreateClientWindow( HINSTANCE ProcessHandle )
    {
        const std::wstring WindowName = L"LOMainWindow";

        WNDCLASS WndClass{};
        WndClass.lpfnWndProc = WindowMessageProcessor;
        WndClass.hInstance = ProcessHandle;
        WndClass.lpszClassName = WindowName.c_str();

        RegisterClass( &WndClass );

        HWND WindowHandle = CreateWindowEx(
            0,								// Optional window styles.
            WindowName.c_str(),				// Window class
            L"Atomic Engine",				// Window text
            WS_OVERLAPPEDWINDOW,			// Window style

            // Size and position
            CW_USEDEFAULT, CW_USEDEFAULT, 1920, 1080,

            NULL,			// Parent window    
            NULL,			// Menu
            ProcessHandle,	// Process handle
            NULL			// Additional application data
        );

        ShowWindow( WindowHandle, SW_SHOWNORMAL );

        RECT WindowRect;
        GetWindowRect( WindowHandle, &WindowRect );

        return WindowHandle;
    }

#if defined( _WIN32 ) || defined( _WIN64 )
    void LoopEngine( HINSTANCE handle )
    {
        HWND hWnd = CreateClientWindow( handle );

        bool shutDown = false;

        World world;

        std::future<void> renderThread = std::async(
            [ & ]()-> void
        {
            DeferredRenderer renderer( hWnd, world.GetScene() );

            renderer.PrepareDefaultResources();

            while ( !shutDown )
            {
                renderer.BeginFrame();
                renderer.RenderFrame();
                renderer.EndFrame();
            }
        } );

        MSG msg{};
        while ( !shutDown )
        {
            while ( PeekMessage( &msg, nullptr, 0, 0, PM_REMOVE ) )
            {
                if ( msg.message == WM_QUIT )
                {
                    shutDown = true;
                    break;
                }
                TranslateMessage( &msg );
                DispatchMessage( &msg );

                InputSystem::GetInstance().ProcessInput( msg, hWnd );
            }

            InputSystem::GetInstance().Tick();

            world.BeginFrame();
            world.Tick();
            world.EndFrame();
        }

        shutDown = true;

        renderThread.wait();
    }
#endif
}

namespace feather
{
    void Launch( FLaunchParam& param )
    {
#if defined( _WIN32 ) || defined( _WIN64 )
        LoopEngine( *(HINSTANCE*)param.platformHandle );
#endif
    }
}
