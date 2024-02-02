#pragma once

struct StaticMeshDataGroup;
struct RawImage;

class AssetLoader
{
public:
	static std::shared_ptr<StaticMeshDataGroup> LoadStaticMeshData( const std::string& fileName );
	static std::shared_ptr<RawImage> LoadRawImage( const std::string& fileName );
};