#include "stdafx.h"
#include "DebugUtil.h"

#include <WinUser.h>

void AEMessageBox( const std::wstring& msg )
{
	MessageBox( nullptr, msg.c_str(), nullptr, MB_ICONERROR );
}