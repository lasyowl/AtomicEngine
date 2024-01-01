#pragma once

#include "ECS.h"
#include "GPIResource_DX12.h"
#include "AtomicEngine.h"
#include "Vector.h"
#include "Matrix.h"
#include "Math.h"
#include "GPIConstantBuffer.h"

struct SceneViewComponent
{
	Vec3 position;
	Vec3 direction;

	Mat4x4 matView;
	Mat4x4 matProjection;
};

class SceneViewSystem : public ISystem
{
public:
	virtual void RunSystem( std::array<std::unique_ptr<struct IComponentRegistry>, NUM_COMPONENT_MAX>& componentRegistry ) override
	{
		ComponentRegistry<SceneViewComponent>* viewCompReg = GetRegistry<SceneViewComponent>( componentRegistry );

		for( Entity entity = 0; entity < NUM_ENTITY_MAX; ++entity )
		{
			if( !viewCompReg->HasComponent( entity ) )
			{
				continue;
			}

			SceneViewComponent& sceneView = viewCompReg->GetComponent( entity );
			static float pos = -10;
			//pos += 0.1f;
			sceneView.position = Vec3{ 0, 0, pos };
			sceneView.direction = Vec3{ 0, 0, 1 };

			sceneView.matView = AEMath::GetViewMatrix( sceneView.position, sceneView.direction, Vec3::up );
			sceneView.matProjection = AEMath::GetPerspectiveMatrixFOV( 0.25f * 3.14f, 1920.0f / 1080.0f, 0.1f, 1000.0f );

			ConstantBuffer constBuffer{};
			constBuffer.matViewProjection = sceneView.matProjection * sceneView.matView;
			AtomicEngine::GetGPI()->UpdateConstantBuffer( constBuffer );
		}
	}
};
