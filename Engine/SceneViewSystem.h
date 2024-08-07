#pragma once

#include "ECS.h"
#include <Core/Vector.h>
#include <Core/Matrix.h>

struct SceneViewComponent
{
	DEFINE_ECS_COMPONENT( SceneView );

	Vec3 position;
	Vec3 direction;

	Mat4x4 matView;
	Mat4x4 matProjection;

	uint32 constBufferID;
};

__declspec( align( 256 ) )
struct SceneViewConstantBuffer
{
	Mat4x4 matViewProjection;
	Mat4x4 matViewProjectionInv;
	Vec3 viewPosition;
};

class SceneViewSystem : public ISystem
{
public:
	DEFINE_ECS_SYSTEM( SceneView );
public:
	virtual void RunSystem( std::array<std::unique_ptr<IComponentRegistry>, NUM_COMPONENT_MAX>& componentRegistry ) override;

private:
	SceneViewConstantBuffer constBuffer;
};
