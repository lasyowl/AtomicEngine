#include "stdafx.h"
#include "AtomicEngine.h"
#include "System.h"
#include "../PrimitiveComponent.h"
#include "../GPI_DX12.h"
#include "../EntityInitializeSystem.h"
#include "../RenderSystem.h"
#include "../KeyInputSystem.h"
#include "../SceneViewSystem.h"

namespace
{
	std::unique_ptr<IGPI> _gpi;

	void InitGPI( HWND hWnd )
	{
		/* todo : Move to somewhere makes sense */
		HMODULE hm = LoadLibrary( L"C:\\Program Files\\Microsoft PIX\\2208.10\\WinPixGpuCapturer.dll" );

		/* todo : Run graphics API by compatibility */

		_gpi = std::make_unique<GPI_DX12>( hWnd, 1920, 1080 );
		_gpi->Initialize();
		_gpi->SetWindowSize( 1920, 1080 );
	}

	LRESULT CALLBACK WindowMessageProcessor( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
	{
		switch( msg )
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

		InitGPI( hWnd );

		ECSAddSystem<EntityInitializeSystem>();
		ECSAddSystem<KeyInputSystem>();

		Entity rootEntity = ECSCreateEntity();
		ECSAddComponent<KeyInputComponent>( rootEntity );
		ECSAddComponent<SceneViewComponent>( rootEntity );
		ECSAddSystem<SceneViewSystem>();

		Entity entity0 = ECSCreateEntityWithMetaData( 0 );
		ECSAddComponent<PrimitiveComponent>( entity0 );

		Entity entity = ECSCreateEntityWithMetaData( 1 );
		ECSAddComponent<PrimitiveComponent>( entity );
		ECSAddSystem<RenderSystem>();

		bool ShutOff = false;

		std::future<void> RenderThread = std::async(
			[ & ]()-> void
			{
				while( !ShutOff )
				{
					_gpi->BeginFrame();

					ECSRunSystems();

					_gpi->EndFrame();
				}
			} );

		MSG msg{};
		while( GetMessage( &msg, NULL, 0, 0 ) > 0 )
		{
			TranslateMessage( &msg );
			DispatchMessage( &msg );
		}

		ShutOff = true;

		RenderThread.wait();
	}
#endif
}

namespace AtomicEngine
{
	void Launch( void* AppEntryParam )
	{
#if defined( _WIN32 ) || defined( _WIN64 )
		LoopEngine( *(HINSTANCE*) AppEntryParam );
#endif
	}

	std::unique_ptr<IGPI>& GetGPI()
	{
		return _gpi;
	}
}
