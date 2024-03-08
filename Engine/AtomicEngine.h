#pragma once

#include "Singleton.h"

class IRHI;

namespace AtomicEngine
{
	void Launch( void* AppEntryParam );

	std::unique_ptr<IRHI>& GetRHI();
};
