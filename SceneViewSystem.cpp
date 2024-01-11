#include "stdafx.h"
#include "SceneViewSystem.h"
#include "KeyInputSystem.h"
#include "GPIResource_DX12.h"
#include "AtomicEngine.h"
#include "GPIConstantBuffer.h"
#include "GPI.h"
#include "Math.h"

void SceneViewSystem::RunSystem( std::array<std::unique_ptr<struct IComponentRegistry>, NUM_COMPONENT_MAX>& componentRegistry )
{
	ComponentRegistry<KeyInputComponent>* keyInputCompReg = GetRegistry<KeyInputComponent>( componentRegistry );
	ComponentRegistry<SceneViewComponent>* viewCompReg = GetRegistry<SceneViewComponent>( componentRegistry );
	if( !viewCompReg || !keyInputCompReg )
	{
		return;
	}

	KeyInputComponent& keyInputComp = keyInputCompReg->GetComponent( 0 );

	for( Entity entity = 0; entity < NUM_ENTITY_MAX; ++entity )
	{
		if( !viewCompReg->HasComponent( entity ) )
		{
			continue;
		}

		SceneViewComponent& sceneView = viewCompReg->GetComponent( entity );

		if( keyInputComp.keyPressed[ KeyType_A ] )
		{
			sceneView.position.x -= 0.1;
		}
		if( keyInputComp.keyPressed[ KeyType_D ] )
		{
			sceneView.position.x += 0.1;
		}
		if( keyInputComp.keyPressed[ KeyType_S ] )
		{
			sceneView.position.y -= 0.1;
		}
		if( keyInputComp.keyPressed[ KeyType_W ] )
		{
			sceneView.position.y += 0.1;
		}
		if( keyInputComp.keyPressed[ KeyType_Q ] )
		{
			sceneView.position.z -= 0.1;
		}
		if( keyInputComp.keyPressed[ KeyType_E ] )
		{
			sceneView.position.z += 0.1;
		}
		sceneView.direction = Vec3{ 0, 0, 1 };

		sceneView.matView = AEMath::GetViewMatrix( sceneView.position, sceneView.position + sceneView.direction, Vec3::up );
		sceneView.matProjection = AEMath::GetPerspectiveMatrixFOV( 0.25f * 3.14f, 1920.0f / 1080.0f, 0.1f, 1000.0f );

		constBuffer.matViewProjection = sceneView.matProjection * sceneView.matView;
		constBuffer.matViewProjectionInv = Mat4x4::Inverse( constBuffer.matViewProjection );
		constBuffer.viewPosition = sceneView.position;
		AtomicEngine::GetGPI()->UpdateConstantBuffer( 0, &constBuffer, sizeof( SceneViewConstantBuffer ) );
	}
}