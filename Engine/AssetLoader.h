#pragma once

struct StaticMeshDataGroup;
struct TextureData;

class AssetLoader
{
public:
	static std::shared_ptr<StaticMeshDataGroup> LoadStaticMeshData( const std::string& fileName );
	static std::shared_ptr<TextureData> LoadTextureData( const std::string& fileName );
};