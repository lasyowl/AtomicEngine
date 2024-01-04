#pragma once

#include "EngineEssential.h"
#include "StaticMesh.h"
#include "RawImage.h"

#include <sstream>
#include <fstream>

class AssetLoader
{
public:
	static std::shared_ptr<StaticMesh> LoadStaticMesh( const std::string& fileName );
	static std::shared_ptr<RawImage> LoadRawImage( const std::string& fileName );
};