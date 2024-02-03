#include "AtomicEngine.h"
#include "System.h"
#include <Engine/PrimitiveComponent.h>
#include <Engine/TransformComponent.h>
#include <Engine/LightComponent.h>
#include <Engine/EntityInitializeSystem.h>
#include <Engine/LightSystem.h>
#include <Engine/RenderSystem.h>
#include <Engine/KeyInputSystem.h>
#include <Engine/SceneViewSystem.h>
#include <GPI/GPI_DX12.h>
#include <Core/IntVector.h>
#include <Engine/AssetLoader.h>

namespace
{
	std::unique_ptr<IGPI> _gpi;

	void InitGPI( HWND hWnd )
	{
		/* todo : Move to somewhere makes sense */
		HMODULE hm = LoadLibrary( L"C:\\Program Files\\Microsoft PIX\\2208.10\\WinPixGpuCapturer.dll" );

		/* todo : Run graphics API by compatibility */

		IVec2 windowSize = IVec2( 1920, 1080 );
		_gpi = std::make_unique<GPI_DX12>( hWnd, windowSize );
		_gpi->Initialize();
		_gpi->SetWindowSize( windowSize );
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

	void InitWithTestScene()
	{
		ECSAddSystem<EntityInitializeSystem>();
		ECSAddSystem<KeyInputSystem>();
		ECSAddSystem<SceneViewSystem>();
		ECSAddSystem<RenderSystem>();

		Entity rootEntity = ECSCreateEntity();
		ECSAddComponent<KeyInputComponent>( rootEntity, nullptr );
		ECSAddComponent<SceneViewComponent>( rootEntity, nullptr );

		//ECSAddSystem<LightSystem>();

		{
			Entity entity0 = ECSCreateEntityWithMetaData( 0 );
			ECSAddComponent<TransformComponent>( entity0, nullptr );

			PrimitiveComponent primComp;
			primComp.staticMeshGroup = StaticMeshCache::AddStaticMeshGroup( "teapot", *AssetLoader::LoadStaticMeshData( "../Resource/teapot.obj" ) );
			ECSAddComponent<PrimitiveComponent>( entity0, &primComp );
		}

		/*Entity entity = ECSCreateEntityWithMetaData( 1 );
		ECSAddComponent<TransformComponent>( entity, nullptr );
		PrimitiveComponent primComp;
		primComp.staticMeshGroup = StaticMeshCache::AddStaticMeshGroup( "sponza", *AssetLoader::LoadStaticMeshData( "../Resource/Sponza-master/sponza.obj" ) );
		ECSAddComponent<PrimitiveComponent>( entity, nullptr );*/

		{
			Entity entityCube = ECSCreateEntityWithMetaData( 2 );
			ECSAddComponent<TransformComponent>( entityCube, nullptr );

			PrimitiveComponent primComp;
			primComp.staticMeshGroup = StaticMeshCache::FindStaticMeshGroup( "sphere" );
			ECSAddComponent<PrimitiveComponent>( entityCube, &primComp );
			ECSAddComponent<LightComponent>( entityCube, nullptr );
		}
	}

#if defined( _WIN32 ) || defined( _WIN64 )
	void LoopEngine( HINSTANCE handle )
	{
		HWND hWnd = CreateClientWindow( handle );

		InitGPI( hWnd );

		InitWithTestScene();

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

		_gpi.reset();
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
