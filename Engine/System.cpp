#include "System.h"

EGraphicsAPI CSystemInfo::GraphicsAPIType = EGraphicsAPI::Windows;

int CSystemInfo::GetPlatformType()
{
#if defined( _WIN32 ) || defined( _WIN64 )
	return Platform_Windows;
#elif defined( __linux__ )
	return Platform_Linux;
#else
	return Platform_Undefined;
#endif
}

//EGraphicsAPI CSystemInfo::GetGraphicsAPIType()
//{
//	return GraphicsAPIType;
//}