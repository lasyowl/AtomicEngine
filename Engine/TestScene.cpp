#include <Engine/TestScene.h>
#include <Engine/PrimitiveComponent.h>
#include <Engine/TransformComponent.h>
#include <Engine/LightComponent.h>
#include <Engine/EntityInitializeSystem.h>
#include <Engine/LightSystem.h>
#include <Engine/RenderSystem.h>
#include <Engine/KeyInputSystem.h>
#include <Engine/SceneViewSystem.h>
#include <Engine/AssetLoader.h>

void InitTestScene()
{
	ECSAddSystem<EntityInitializeSystem>();
	ECSAddSystem<KeyInputSystem>();
	ECSAddSystem<SceneViewSystem>();
	ECSAddSystem<RenderSystem>();

	Entity rootEntity = ECSCreateEntity();
	ECSAddComponent<KeyInputComponent>( rootEntity, nullptr );
	ECSAddComponent<SceneViewComponent>( rootEntity, nullptr );

	//ECSAddSystem<LightSystem>();

	{
		Entity entity0 = ECSCreateEntity();
		TransformComponent transformComp{};
		transformComp.scale = Vec3( 1.0f, 1.0f, 1.0f );
		ECSAddComponent<TransformComponent>( entity0, &transformComp );

		PrimitiveComponent primComp;
		primComp.staticMeshGroup = StaticMeshCache::AddStaticMeshGroup( "teapot", *AssetLoader::LoadStaticMeshData( "../Resource/teapot.obj" ) );
		primComp.material.baseColor = Vec3::right;
		primComp.material.metalness = 0.75f;
		primComp.material.roughness = 0.7f;
		ECSAddComponent<PrimitiveComponent>( entity0, &primComp );
	}

	{
		Entity entityCube = ECSCreateEntity();
		TransformComponent transformComp{};
		transformComp.position = Vec3( 3.0f, 2.0f, -2.0f );
		transformComp.scale = Vec3::one;
		ECSAddComponent<TransformComponent>( entityCube, &transformComp );

		PrimitiveComponent primComp;
		primComp.staticMeshGroup = StaticMeshCache::FindStaticMeshGroup( "sphere" );
		primComp.material.baseColor = Vec3::front;
		primComp.material.metalness = 0.2f;
		primComp.material.roughness = 0.5f;
		ECSAddComponent<PrimitiveComponent>( entityCube, &primComp );
		ECSAddComponent<LightComponent>( entityCube, nullptr );
	}

	{ // bottom cover
		Entity entity = ECSCreateEntity();
		TransformComponent transformComp{};
		transformComp.position = Vec3( 0.0f, 0.0f, 0.0f );
		transformComp.scale = Vec3( 10.0f, 10.0f, 10.0f );
		ECSAddComponent<TransformComponent>( entity, &transformComp );
		PrimitiveComponent primComp;
		primComp.staticMeshGroup = StaticMeshCache::FindStaticMeshGroup( "plane" );
		primComp.material.baseColor = Vec3::one;
		primComp.material.metalness = 0.9f;
		primComp.material.roughness = 0.1f;
		ECSAddComponent<PrimitiveComponent>( entity, &primComp );
	}

	{ // left cover
		Entity entity = ECSCreateEntity();
		TransformComponent transformComp{};
		transformComp.position = Vec3( -9.9f, 9.9f, 0.0f );
		transformComp.rotation = Vec3( 0.0f, 0.0f, -3.14f / 2.0f );
		transformComp.scale = Vec3( 10.0f, 10.0f, 10.0f );
		ECSAddComponent<TransformComponent>( entity, &transformComp );
		PrimitiveComponent primComp;
		primComp.staticMeshGroup = StaticMeshCache::FindStaticMeshGroup( "plane" );
		primComp.material.baseColor = Vec3( 1, 0, 0 );
		primComp.material.metalness = 0.5f;
		primComp.material.roughness = 1.0f;
		ECSAddComponent<PrimitiveComponent>( entity, &primComp );
	}

	{ // top cover
		Entity entity = ECSCreateEntity();
		TransformComponent transformComp{};
		transformComp.position = Vec3( 0.0f, 19.9f, 0.0f );
		transformComp.rotation = Vec3( 0.0f, 0.0f, 3.14f );
		transformComp.scale = Vec3( 10.0f, 10.0f, 10.0f );
		ECSAddComponent<TransformComponent>( entity, &transformComp );
		PrimitiveComponent primComp;
		primComp.staticMeshGroup = StaticMeshCache::FindStaticMeshGroup( "plane" );
		primComp.material.baseColor = Vec3( 1, 1, 1 );
		primComp.material.metalness = 0.5f;
		primComp.material.roughness = 1.0f;
		ECSAddComponent<PrimitiveComponent>( entity, &primComp );
	}

	{ // right cover
		Entity entity = ECSCreateEntity();
		TransformComponent transformComp{};
		transformComp.position = Vec3( 9.9f, 9.9f, 0.0f );
		transformComp.rotation = Vec3( 0.0f, 0.0f, 3.14f / 2.0f );
		transformComp.scale = Vec3( 10.0f, 10.0f, 10.0f );
		ECSAddComponent<TransformComponent>( entity, &transformComp );
		PrimitiveComponent primComp;
		primComp.staticMeshGroup = StaticMeshCache::FindStaticMeshGroup( "plane" );
		primComp.material.baseColor = Vec3( 0, 0, 1 );
		primComp.material.metalness = 0.5f;
		primComp.material.roughness = 1.0f;
		ECSAddComponent<PrimitiveComponent>( entity, &primComp );
	}

	{ // back cover
		Entity entity = ECSCreateEntity();
		TransformComponent transformComp{};
		transformComp.position = Vec3( 0.0f, 9.9f, 9.9f );
		transformComp.rotation = Vec3( -3.14f / 2.0f, 0.0f, 0.0f );
		transformComp.scale = Vec3( 10.0f, 10.0f, 10.0f );
		ECSAddComponent<TransformComponent>( entity, &transformComp );
		PrimitiveComponent primComp;
		primComp.staticMeshGroup = StaticMeshCache::FindStaticMeshGroup( "plane" );
		primComp.material.baseColor = Vec3( 0, 1, 0 );
		primComp.material.metalness = 0.5f;
		primComp.material.roughness = 1.0f;
		ECSAddComponent<PrimitiveComponent>( entity, &primComp );
	}

	/*for( uint32 row = 0; row < 4; ++row )
	{
		for( uint32 col = 0; col < 4; ++col )
		{
			Entity entityCube = ECSCreateEntity();
			TransformComponent transformComp{};
			transformComp.position = Vec3( col * 4, row * 4, 50.0f );
			transformComp.scale = Vec3::one;
			ECSAddComponent<TransformComponent>( entityCube, &transformComp );

			PrimitiveComponent primComp;
			primComp.staticMeshGroup = StaticMeshCache::FindStaticMeshGroup( "sphere" );
			primComp.material.baseColor = Vec3::front;
			primComp.material.metalness = 0.001f + row * 0.333f;
			primComp.material.roughness = 0.001f + col * 0.333f;
			ECSAddComponent<PrimitiveComponent>( entityCube, &primComp );
		}
	}*/
}