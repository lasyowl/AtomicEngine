#include "stdafx.h"
#include "AtomicEngine.h"

int WINAPI WinMain( _In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ PSTR lpCmdLine, _In_ int nCmdShow )
{
	AtomicEngine::Launch( &hInstance );

	return 0;
}