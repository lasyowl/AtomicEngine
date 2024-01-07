#include "stdafx.h"
#include "SceneViewSystem.h"
#include "GPIResource_DX12.h"
#include "AtomicEngine.h"
#include "GPIConstantBuffer.h"
#include "GPI.h"
#include "Math.h"

void SceneViewSystem::RunSystem( std::array<std::unique_ptr<struct IComponentRegistry>, NUM_COMPONENT_MAX>& componentRegistry )
{
	ComponentRegistry<SceneViewComponent>* viewCompReg = GetRegistry<SceneViewComponent>( componentRegistry );
	if( !viewCompReg )
	{
		return;
	}

	for( Entity entity = 0; entity < NUM_ENTITY_MAX; ++entity )
	{
		if( !viewCompReg->HasComponent( entity ) )
		{
			continue;
		}

		SceneViewComponent& sceneView = viewCompReg->GetComponent( entity );
		static float posZ = -10;
		static float posY = 0;
		//pos += 0.1f;
		posY = min( posY + 0.01f, 10 );
		sceneView.position = Vec3{ 5, posY, posZ };
		sceneView.direction = Vec3{ 0, 0, 1 };

		sceneView.matView = AEMath::GetViewMatrix( sceneView.position, sceneView.direction, Vec3::up );
		sceneView.matProjection = AEMath::GetPerspectiveMatrixFOV( 0.25f * 3.14f, 1920.0f / 1080.0f, 0.1f, 1000.0f );

		ConstantBuffer constBuffer{};
		constBuffer.matViewProjection = sceneView.matProjection * sceneView.matView;
		AtomicEngine::GetGPI()->UpdateConstantBuffer( constBuffer );
	}
}