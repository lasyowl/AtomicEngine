#pragma once

#include "EngineEssential.h"

class IRenderPass
{
public:
	virtual void Dispatch() abstract;
};

class CRenderPassTest : public IRenderPass
{
	virtual void Dispatch() override
	{

	}
};