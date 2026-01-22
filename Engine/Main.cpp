#include "FeatherEntry.h"

int WINAPI WinMain( _In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ PSTR lpCmdLine, _In_ int nCmdShow )
{
    feather::FLaunchParam param{ .platformHandle = &hInstance };

    feather::Launch( param );

    return 0;
}