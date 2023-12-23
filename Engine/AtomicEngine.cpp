#include "stdafx.h"
#include "AtomicEngine.h"
#include "RSClient.h"
#include "System.h"
#include "../GPI_DX12.h"

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
	}

	LRESULT CALLBACK WindowMessageProcessor( HWND HWnd, UINT Msg, WPARAM WParam, LPARAM LParam )
	{
		switch( Msg )
		{
		case WM_DESTROY:
		{
			PostQuitMessage( 0 );
			return 0;
		}
		//case WM_PAINT:
		//{
		//	PAINTSTRUCT ps;
		//	HDC hdc = BeginPaint( HWnd, &ps );

		//	// All painting occurs here, between BeginPaint and EndPaint.

		//	FillRect( hdc, &ps.rcPaint, (HBRUSH) (COLOR_WINDOW + 2) );

		//	EndPaint( HWnd, &ps );

		//	return 0;
		//}
		}
		return DefWindowProc( HWnd, Msg, WParam, LParam );
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
			L"RSClient",					// Window text
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

		bool ShutOff = false;

		std::future<void> RenderThread = std::async(
			[&]()-> void
			{
				while( !ShutOff )
				{
					//RenderFrame();
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
