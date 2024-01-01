#pragma once

#include "EngineEssential.h"
#include "StaticMesh.h"

#include <sstream>
#include <fstream>

class AssetLoader
{
public:
	static std::shared_ptr<StaticMesh> LoadStaticMesh( const std::string& fileName );
};