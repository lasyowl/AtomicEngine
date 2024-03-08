#include <Engine/AtomicEngine.h>
#include <RHI/RHI_DX12.h>
#include <Core/IntVector.h>
#include <Engine/TestScene.h>
#include <Engine/ECS.h>

namespace
{
	std::unique_ptr<IRHI> _RHI;

	void InitRHI( HWND hWnd )
	{
		/* todo : Move to somewhere makes sense */
		HMODULE hm = LoadLibrary( L"C:\\Program Files\\Microsoft PIX\\2312.08\\WinPixGpuCapturer.dll" );

		/* todo : Run graphics API by compatibility */

		IVec2 windowSize = IVec2( 1920, 1080 );
		_RHI = std::make_unique<RHI_DX12>( hWnd, windowSize );
		_RHI->Initialize();
		_RHI->SetWindowSize( windowSize );
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

		InitRHI( hWnd );

		InitTestScene();

		bool ShutOff = false;

		std::future<void> ecsThread = std::async(
			[ & ]()-> void
			{
				while( !ShutOff )
				{
					ECSRunSystems();
				}
			} );

		MSG msg{};
		while( GetMessage( &msg, NULL, 0, 0 ) > 0 )
		{
			TranslateMessage( &msg );
			DispatchMessage( &msg );
		}

		ShutOff = true;

		ecsThread.wait();

		_RHI.reset();
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

	std::unique_ptr<IRHI>& GetRHI()
	{
		return _RHI;
	}
}
