#pragma once

#include "Singleton.h"

class IGPI;

namespace AtomicEngine
{
	void Launch( void* AppEntryParam );

	std::unique_ptr<IGPI>& GetGPI();
};
