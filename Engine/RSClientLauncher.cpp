#include "stdafx.h"
#include "RSClientLauncher.h"
#include "RSClient.h"
#include "System.h"

void CRSClientLauncher::Launch( void* AppEntryParam )
{
	std::shared_ptr<IRSClient> Client = nullptr;

	switch ( CSystemInfo::GetPlatformType() )
	{
		case Platform_Windows:	Client = std::make_shared<CRSClientWindows>();	break;
		case Platform_Linux:	Client = std::make_shared<CRSClientLinux>();	break;
	}

	Client->Launch( AppEntryParam );
}
