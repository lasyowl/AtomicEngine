#pragma once

#include "Singleton.h"

class CRSClientLauncher : public TSingleton<CRSClientLauncher>
{
public:
	void Launch( void* AppEntryParam );
};