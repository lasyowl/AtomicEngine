#include "stdafx.h"
#include "RSClient.h"
#include "System.h"
#include "SystemTranslator.h"
#include "RenderObjectBatch.h"
#include "GraphicsInterfaceDX12.h"

std::shared_ptr<IGraphicsInterface> gGraphics;
CRenderObjectBatch gROS;

void CRSClientWindows::InitGraphics()
{
	/* todo : Move to somewhere reasonable */
	HMODULE hm = LoadLibrary( L"C:\\Program Files\\Microsoft PIX\\2208.10\\WinPixGpuCapturer.dll" );

	/* todo : Run graphics API by compatibility */

	SGraphicsInitParamDX12 Param;
	Param.WindowHandle = WindowHandle;
	Param.WindowRect = Convert::ToEngineRect( WindowRect );

	gGraphics = std::make_shared<CGraphicsInterfaceDX12>();
	gGraphics->InitGraphics( Param );

	CRenderObjectRef RO( new CRenderObject() );
	RO->InitRenderResource();

	gROS.SetPipelineState( gGraphics->CreatePipelineState() );
	gROS.AddRenderObject( RO );
}

void CRSClientWindows::RenderFrame()
{
	gGraphics->BeginFrame();

	Renderer.Render( gROS );

	gGraphics->EndFrame();
}

void CRSClientWindows::Launch( void* AppEntryParam )
{
	CreateClientWindow( *( HINSTANCE* ) AppEntryParam );
	
	InitGraphics();

	bool ShutOff = false;

	std::future<void> RenderThread = std::async(
		[&]()-> void
		{
			while ( !ShutOff )
			{
				RenderFrame();
			}
		} );

	MSG msg{};
	while ( GetMessage( &msg, NULL, 0, 0 ) > 0 )
	{
		TranslateMessage( &msg );
		DispatchMessage( &msg );
	}

	ShutOff = true;

	RenderThread.wait();
}

void CRSClientWindows::CreateClientWindow( HINSTANCE ProcessHandle )
{
	const std::wstring WindowName = L"LOMainWindow";

	WNDCLASS WndClass{};
	WndClass.lpfnWndProc = WindowMessageProcessor;
	WndClass.hInstance = ProcessHandle;
	WndClass.lpszClassName = WindowName.c_str();

	RegisterClass( &WndClass );

	WindowHandle = CreateWindowEx(
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

	GetWindowRect( WindowHandle, &WindowRect );
}

LRESULT CALLBACK CRSClientWindows::WindowMessageProcessor( HWND HWnd, UINT Msg, WPARAM WParam, LPARAM LParam )
{
	switch ( Msg )
	{
		case WM_DESTROY:
		{
			PostQuitMessage( 0 );
			return 0;
		}
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint( HWnd, &ps );

			// All painting occurs here, between BeginPaint and EndPaint.

			FillRect( hdc, &ps.rcPaint, ( HBRUSH ) ( COLOR_WINDOW + 2 ) );

			EndPaint( HWnd, &ps );

			return 0;
		}
	}
	return DefWindowProc( HWnd, Msg, WParam, LParam );
}