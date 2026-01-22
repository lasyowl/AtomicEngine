#pragma once

#include <Renderer/Scene.h>

class IEngineObject;
class SceneObject;

class World
{
public:
    World();
    ~World();

public:
    void BeginFrame();
    void EndFrame();
    void Tick();

    Scene& GetScene() { return _scene; }

private:
    template<typename T, typename... Args>
    T* AddSceneObject( Args&&... args );

    void RemoveSceneObject( uint32 registryIndex );

    template<typename T, typename... Args>
    requires std::is_base_of_v<IEngineObject, T>
    T* AddEngineObject( Args&&... args );

private:
    Scene _scene;

    std::vector<IEngineObject*> _engineObjects;
    std::vector<SceneObject*> _sceneObjects;
};