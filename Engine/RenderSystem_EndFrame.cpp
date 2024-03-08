#include "RenderSystem.h"
#include "AtomicEngine.h"
#include <RHI/RHI.h>
#include <RHI/RHIPipeline.h>

void RenderSystem::EndFrame( std::array<std::unique_ptr<IComponentRegistry>, NUM_COMPONENT_MAX>& componentRegistry )
{
	uint32 swapChainIndex = AtomicEngine::GetRHI()->GetSwapChainCurrentIndex();
	AtomicEngine::GetRHI()->EndFrame( *_swapChainResource[ swapChainIndex ] );
}