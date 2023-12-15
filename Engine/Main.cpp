#include "stdafx.h"
#include "RSClientLauncher.h"
#include "../TestSystem.h"

int WINAPI WinMain( _In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ PSTR lpCmdLine, _In_ int nCmdShow )
{
	Entity entity = ECSCreateEntity();
	ECSAddComponent<STestComponent>( entity );
	ECSAddSystem<CTestSystem>();

	ECSRunSystems();
	ECSRunSystems();

	//CRSClientLauncher::GetInstance().Launch( &hInstance );

	return 0;
}