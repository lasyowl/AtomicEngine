#pragma once

#include "EngineEssential.h"
#include "Singleton.h"

class IGPI;

namespace AtomicEngine
{
	void Launch( void* AppEntryParam );

	std::unique_ptr<IGPI>& GetGPI();
};
