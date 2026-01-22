#include "World.h"

#include <Engine/AssetLoader.h>
#include <Engine/EngineObject.h>
#include <Engine/ViewController.h>
#include <Renderer/Entity.h>
#include <Renderer/RenderCommand.h>

World::World()
{

}

World::~World()
{

}

void World::BeginFrame()
{
    // @TODO: Remove test code
    static bool bTest = true;
    if ( bTest )
    {
        std::vector<std::shared_ptr<MeshData>> meshArray = AssetLoader::LoadMeshData( "../Resource/Sponza-master/sponza.obj" );
        //std::shared_ptr<MeshData> meshData = AssetLoader::LoadMeshData( "../Resource/teapot.obj" );

        for ( auto& meshData : meshArray )
        {
            MeshObject* meshObject = AddSceneObject<MeshObject>( meshData, Vec3::zero );
            MeshEntity* meshEntity = static_cast<MeshEntity*>( meshObject->CreateEntity() );

            RenderUtil::EnqueueRenderCommand( [ &scene = _scene, meshEntity ]( IRenderBackend& rb )
            {
                scene.AddMeshEntity( meshEntity );
            } );
        }

        ViewObject* viewObject = AddSceneObject<ViewObject>( Vec3( 0, 0, -40 ) );
        ViewEntity* viewEntity = static_cast<ViewEntity*>( viewObject->CreateEntity() );

        IEngineObject* viewController = AddEngineObject<ViewController>( *viewObject );

        RenderUtil::EnqueueRenderCommand( [ &scene = _scene, viewEntity ]( IRenderBackend& rb )
        {
            scene.AddViewEntity( viewEntity );
        } );

        bTest = false;
    }
}

void World::EndFrame()
{
    RenderUtil::PublishRenderCommands();
}

void World::Tick()
{
    for ( int32 index = 0; index < _sceneObjects.size(); ++index )
    {
        SceneObject* sceneObject = _sceneObjects[ index ];
        sceneObject->Tick();

        if ( sceneObject->IsRenderStateDirty() )
        {
        }
    }

    for ( int32 index = 0; index < _engineObjects.size(); ++index )
    {
        IEngineObject* engineObject = _engineObjects[ index ];
        engineObject->Tick();
    }
}

template<typename T, typename... Args>
T* World::AddSceneObject( Args&&... args )
{
    T* sceneObject = new T( std::forward<Args>( args )... );
    _sceneObjects.emplace_back( sceneObject );

    return sceneObject;
}

void World::RemoveSceneObject( uint32 registryIndex )
{
    std::swap( _sceneObjects[ registryIndex ], _sceneObjects.back() );

    delete _sceneObjects.back();
    _sceneObjects.pop_back();
}

template<typename T, typename... Args>
requires std::is_base_of_v<IEngineObject, T>
T* World::AddEngineObject( Args&&... args )
{
    T* engineObject = new T( std::forward<Args>( args )... );
    _engineObjects.emplace_back( engineObject );
    return engineObject;
}
