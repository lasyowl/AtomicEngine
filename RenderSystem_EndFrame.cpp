#include "stdafx.h"
#include "RenderSystem.h"
#include "AtomicEngine.h"
#include "GPI.h"
#include "GPIPipeline.h"

void RenderSystem::EndFrame( std::array<std::unique_ptr<IComponentRegistry>, NUM_COMPONENT_MAX>& componentRegistry )
{
	uint32 swapChainIndex = AtomicEngine::GetGPI()->GetSwapChainCurrentIndex();
	AtomicEngine::GetGPI()->EndFrame( *_swapChainResource[ swapChainIndex ] );
}