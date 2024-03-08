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
	SceneViewComponent sceneViewComp{};
	sceneViewComp.position = Vec3( -5, 13, -15 );
	sceneViewComp.direction = Vec3( 0.5f, -0.7f, 1 );
	ECSAddComponent<SceneViewComponent>( rootEntity, &sceneViewComp );

	//ECSAddSystem<LightSystem>();

	{
		Entity entity = ECSCreateEntity();
		TransformComponent transformComp{};
		transformComp.scale = Vec3( 1.0f, 1.0f, 1.0f );
		ECSAddComponent<TransformComponent>( entity, &transformComp );

		PrimitiveComponent primComp;
		primComp.staticMeshGroup = StaticMeshCache::AddStaticMeshGroup( "teapot", *AssetLoader::LoadStaticMeshData( "../Resource/teapot.obj" ) );
		primComp.material.constants.baseColor = Vec3::right;
		primComp.material.constants.metalness = 0.75f;
		primComp.material.constants.roughness = 0.7f;
		ECSAddComponent<PrimitiveComponent>( entity, &primComp );
	}

	{
		Entity entity = ECSCreateEntity();
		TransformComponent transformComp{};
		transformComp.position = Vec3( 3.0f, 2.0f, -2.0f );
		transformComp.scale = Vec3::one;
		ECSAddComponent<TransformComponent>( entity, &transformComp );

		PrimitiveComponent primComp;
		primComp.staticMeshGroup = StaticMeshCache::FindStaticMeshGroup( "sphere" );
		primComp.material.constants.baseColor = Vec3::front;
		primComp.material.constants.metalness = 0.2f;
		primComp.material.constants.roughness = 0.5f;
		ECSAddComponent<PrimitiveComponent>( entity, &primComp );
		ECSAddComponent<LightComponent>( entity, nullptr );
	}

	{
		Entity entity = ECSCreateEntity();
		TransformComponent transformComp{};
		transformComp.position = Vec3( -8.0f, 1.0f, 8.0f );
		transformComp.scale = Vec3::one;
		ECSAddComponent<TransformComponent>( entity, &transformComp );

		PrimitiveComponent primComp;
		primComp.staticMeshGroup = StaticMeshCache::FindStaticMeshGroup( "cube" );
		primComp.material.constants.baseColor = Vec3::front;
		primComp.material.constants.metalness = 0.5f;
		primComp.material.constants.roughness = 1.0f;
		//primComp.material2.baseColorResource = 
		ECSAddComponent<PrimitiveComponent>( entity, &primComp );
	}

	{ // bottom cover
		Entity entity = ECSCreateEntity();
		TransformComponent transformComp{};
		transformComp.position = Vec3( 0.0f, 0.0f, 0.0f );
		transformComp.scale = Vec3( 10.0f, 10.0f, 10.0f );
		ECSAddComponent<TransformComponent>( entity, &transformComp );
		PrimitiveComponent primComp;
		primComp.staticMeshGroup = StaticMeshCache::FindStaticMeshGroup( "plane" );
		primComp.material.constants.baseColor = Vec3::one;
		primComp.material.constants.metalness = 1.0f;
		primComp.material.constants.roughness = 0.6f;
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
		primComp.material.constants.baseColor = Vec3( 1, 0, 0 );
		primComp.material.constants.metalness = 0.5f;
		primComp.material.constants.roughness = 1.0f;
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
		primComp.material.constants.baseColor = Vec3( 1, 1, 1 );
		primComp.material.constants.metalness = 0.5f;
		primComp.material.constants.roughness = 1.0f;
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
		primComp.material.constants.baseColor = Vec3( 0, 0, 1 );
		primComp.material.constants.metalness = 0.5f;
		primComp.material.constants.roughness = 1.0f;
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
		primComp.material.constants.baseColor = Vec3( 0, 1, 0 );
		primComp.material.constants.metalness = 0.5f;
		primComp.material.constants.roughness = 1.0f;
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