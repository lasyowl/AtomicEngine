#pragma once

#include <Renderer/View.h>
#include <Renderer/Mesh.h>

class Scene
{
public:
    Scene();

    void AddViewEntity( ViewEntity* entity ) { _viewEntities.push_back( entity ); }
    void AddMeshEntity( MeshEntity* entity ) { _meshEntities.push_back( entity ); }

    const std::vector<ViewEntity*>& GetViewEntities() { return _viewEntities; }
    const std::vector<MeshEntity*>& GetMeshEntities() { return _meshEntities; }

private:
    std::vector<ViewEntity*> _viewEntities;
    std::vector<MeshEntity*> _meshEntities;
};
