#pragma once

struct MeshData;
class Texture;
class Material;

class AssetLoader
{
public:
	static std::vector<std::shared_ptr<MeshData>> LoadMeshData( const std::string& fileName );
	static std::shared_ptr<Texture> LoadTexture( const std::string& fileName );
    static std::shared_ptr<Material> LoadMaterial( const std::string& fileName );
};