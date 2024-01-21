#include "stdafx.h"
#include "EntityInitializeSystem.h"
#include "AssetLoader.h"
#include "MetaDataComponent.h"
#include "TransformComponent.h"
#include "PrimitiveComponent.h"
#include "LightComponent.h"
#include "SampleMesh.h"

void EntityInitializeSystem::RunSystem( std::array<std::unique_ptr<struct IComponentRegistry>, NUM_COMPONENT_MAX>& componentRegistry )
{
	ComponentRegistry<MetaDataComponent>* metaDataCompReg = GetRegistry<MetaDataComponent>( componentRegistry );
	ComponentRegistry<TransformComponent>* transformCompReg = GetRegistry<TransformComponent>( componentRegistry );
	ComponentRegistry<PrimitiveComponent>* primitiveCompReg = GetRegistry<PrimitiveComponent>( componentRegistry );
	ComponentRegistry<LightComponent>* lightCompReg = GetRegistry<LightComponent>( componentRegistry );
	if( !metaDataCompReg || !transformCompReg || !primitiveCompReg || !lightCompReg )
	{
		return;
	}

	for( Entity entity = 0; entity < NUM_ENTITY_MAX; ++entity )
	{
		if( !metaDataCompReg->HasComponent( entity ) )
		{
			continue;
		}

		MetaDataComponent& metaDataComp = metaDataCompReg->GetComponent( entity );
		if( metaDataComp.bLoaded )
		{
			continue;
		}

		if( primitiveCompReg->HasComponent( entity ) )
		{
			TransformComponent& transformComp = transformCompReg->GetComponent( entity );
			PrimitiveComponent& primitiveComp = primitiveCompReg->GetComponent( entity );
			if( metaDataComp.hash == 0 )
			{
				primitiveComp.staticMeshData = AssetLoader::LoadStaticMeshData( "Resource/teapot.obj" );
			}
			else if( metaDataComp.hash == 1 )
			{
				primitiveComp.staticMeshData = std::make_shared<StaticMeshData>( SampleMesh::GetQuad() );
				transformComp.scale = Vec3( 2.0f, 2.0f, 2.0f );
			}
			else if( metaDataComp.hash == 2 )
			{
				primitiveComp.staticMeshData = std::make_shared<StaticMeshData>( SampleMesh::GetSphere() );
				transformComp.position = Vec3( 1, 0, 0 );
			}
			else if( metaDataComp.hash == 3 )
			{
				primitiveComp.staticMeshData = std::make_shared<StaticMeshData>( SampleMesh::GetQuad() );
			}
			else if( metaDataComp.hash == 4 )
			{
				primitiveComp.staticMeshData = std::make_shared<StaticMeshData>( SampleMesh::GetQuad() );
			}
		}

		if( lightCompReg->HasComponent( entity ) )
		{
			LightComponent& lightComp = lightCompReg->GetComponent( entity );
			if( metaDataComp.hash == 2 )
			{
				lightComp.intensity = 1.0f;
			}
		}

		metaDataComp.bLoaded = true;
	}
}