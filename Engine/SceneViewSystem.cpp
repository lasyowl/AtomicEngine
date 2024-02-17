#include "SceneViewSystem.h"
#include "KeyInputSystem.h"
#include "AtomicEngine.h"
#include <Core/Math.h>

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

		const float camMoveSpeed = 0.3f;
		const float camRotateSpeed = 0.03f;

		SceneViewComponent& sceneView = viewCompReg->GetComponent( entity );

		if( keyInputComp.keyPressed[ KeyType_A ] )
		{
			sceneView.position.x -= camMoveSpeed;
		}
		if( keyInputComp.keyPressed[ KeyType_D ] )
		{
			sceneView.position.x += camMoveSpeed;
		}
		if( keyInputComp.keyPressed[ KeyType_S ] )
		{
			sceneView.position.z -= camMoveSpeed;
		}
		if( keyInputComp.keyPressed[ KeyType_W ] )
		{
			sceneView.position.z += camMoveSpeed;
		}
		if( keyInputComp.keyPressed[ KeyType_Q ] )
		{
			sceneView.position.y -= camMoveSpeed;
		}
		if( keyInputComp.keyPressed[ KeyType_E ] )
		{
			sceneView.position.y += camMoveSpeed;
		}
		if( keyInputComp.keyPressed[ KeyType_J ] )
		{
			sceneView.direction.x -= camRotateSpeed;
		}
		if( keyInputComp.keyPressed[ KeyType_L ] )
		{
			sceneView.direction.x += camRotateSpeed;
		}
		if( keyInputComp.keyPressed[ KeyType_I ] )
		{
			sceneView.direction.y += camRotateSpeed;
		}
		if( keyInputComp.keyPressed[ KeyType_K ] )
		{
			sceneView.direction.y -= camRotateSpeed;
		}
		sceneView.direction.Normalize();

		sceneView.matView = AEMath::GetViewMatrix( sceneView.position, sceneView.position + sceneView.direction, Vec3::up );
		sceneView.matProjection = AEMath::GetPerspectiveMatrixFOV( 0.25f * 3.14f, 1920.0f / 1080.0f, 0.1f, 5000.0f );
	}
}