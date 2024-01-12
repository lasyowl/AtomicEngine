#include "stdafx.h"
#include "EntityInitializeSystem.h"
#include "AssetLoader.h"
#include "MetaDataComponent.h"
#include "PrimitiveComponent.h"
#include "SampleMesh.h"

void EntityInitializeSystem::RunSystem( std::array<std::unique_ptr<struct IComponentRegistry>, NUM_COMPONENT_MAX>& componentRegistry )
{
	ComponentRegistry<MetaDataComponent>* metaDataCompReg = GetRegistry<MetaDataComponent>( componentRegistry );
	ComponentRegistry<PrimitiveComponent>* primitiveCompReg = GetRegistry<PrimitiveComponent>( componentRegistry );
	if( !metaDataCompReg || !primitiveCompReg )
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
			PrimitiveComponent& primitiveComp = primitiveCompReg->GetComponent( entity );
			if( metaDataComp.hash == 0 )
			{
				primitiveComp.staticMesh = AssetLoader::LoadStaticMesh( "Resource/teapot.obj" );
			}
			else if( metaDataComp.hash == 1 )
			{
				primitiveComp.staticMesh = std::make_shared<StaticMesh>( SampleMesh::GetQuad() );
				primitiveComp.scale = Vec3( 2.0f, 2.0f, 2.0f );
			}
			else if( metaDataComp.hash == 2 )
			{
				primitiveComp.staticMesh = std::make_shared<StaticMesh>( SampleMesh::GetCube() );
			}
			else if( metaDataComp.hash == 3 )
			{
				primitiveComp.staticMesh = std::make_shared<StaticMesh>( SampleMesh::GetQuad() );
			}
			else if( metaDataComp.hash == 4 )
			{
				primitiveComp.staticMesh = std::make_shared<StaticMesh>( SampleMesh::GetQuad() );
			}
		}

		metaDataComp.bLoaded = true;
	}
}