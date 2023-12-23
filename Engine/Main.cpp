#include "stdafx.h"
#include "AtomicEngine.h"
#include "../TestSystem.h"

int WINAPI WinMain( _In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ PSTR lpCmdLine, _In_ int nCmdShow )
{
	Entity entity = ECSCreateEntity();
	ECSAddComponent<STestComponent>( entity );
	ECSAddSystem<CTestSystem>();

	ECSRunSystems();
	ECSRunSystems();

	AtomicEngine::Launch( &hInstance );

	return 0;
}