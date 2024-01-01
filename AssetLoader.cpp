#include "stdafx.h"
#include "AssetLoader.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

std::shared_ptr<StaticMesh> LoadStaticMesh_WavefrontObj( std::ifstream& file )
{
	std::shared_ptr<StaticMesh> staticMesh = std::make_shared<StaticMesh>();

	std::vector<Vec3>& position = staticMesh->position;
	std::vector<Vec2>& uv = staticMesh->uv;
	std::vector<std::vector<uint32>>& index = staticMesh->indices;
	std::vector<uint32>& currentIndex = index.emplace_back();

	std::string line;
	while( std::getline( file, line ) )
	{
		std::stringstream stream( line );
		std::string header;
		stream >> header;

		constexpr uint32 NUM_VERTEX = 3;
		constexpr uint32 NUM_UV = 2;

		if( header.compare( "o" ) == 0 )
		{
			if( !staticMesh->name.empty() ) break; // temp
			stream >> staticMesh->name;
		}
		else if( header.compare( "v" ) == 0 )
		{
			float buffer[ 3 ];
			for( uint32 iter = 0; iter < NUM_VERTEX; ++iter )
			{
				stream >> buffer[ iter ];
			}
			Vec3 buffer1 = { buffer[ 0 ], buffer[ 1 ], buffer[ 2 ] };
			position.emplace_back( buffer1 );
		}
		else if( header.compare( "vt" ) == 0 )
		{
			float buffer[ 2 ];
			for( uint32 iter = 0; iter < NUM_UV; ++iter )
			{
				stream >> buffer[ iter ];
			}
			Vec2 buffer1 = { buffer[ 0 ], buffer[ 1 ] };
			uv.emplace_back( buffer1 );
		}
		else if( header.compare( "vn" ) == 0 )
		{
		}
		else if( header.compare( "g" ) == 0 )
		{

		}
		else if( header.compare( "f" ) == 0 )
		{
			for( uint32 iter = 0; iter < NUM_VERTEX; ++iter )
			{
				std::string lineBuffer;
				stream >> lineBuffer;

				uint32 vertexIndex = 0;
				uint32 uvIndex = 0;

				std::stringstream vertexStream( lineBuffer );
				std::string buffer;
				if( std::getline( vertexStream, buffer, '/' ) )
				{
					vertexIndex = std::stoi( buffer ) - 1;
					uvIndex = vertexIndex;
				}
				if( std::getline( vertexStream, buffer, '/' ) )
				{
					uvIndex = std::stoi( buffer ) - 1;
				}

				uint32 indexMax = max( vertexIndex, uvIndex );
				if( position.size() <= indexMax )
				{
					position.resize( indexMax + 1 );
				}

				if( vertexIndex != uvIndex )
				{
					position[ uvIndex ] = position[ vertexIndex ];
				}

				currentIndex.emplace_back( uvIndex );
			}
		}
	}

	return staticMesh;
}

std::shared_ptr<StaticMesh> LoadStaticMesh_Assimp( const aiScene* scene )
{
	std::shared_ptr<StaticMesh> staticMesh = std::make_shared<StaticMesh>();

	const aiMesh* mesh = scene->mMeshes[ 0 ];

	staticMesh->position.resize( mesh->mNumVertices );
	//staticMesh->uv.resize( mesh->mNumVertices );
	staticMesh->indices.resize( 1 );
	std::vector<uint32>& indices = staticMesh->indices[ 0 ];
	indices.resize( 3 * mesh->mNumFaces );

	for( int32 index = 0; index < mesh->mNumFaces; ++index )
	{
		const aiFace& face = mesh->mFaces[ index ];

		indices[ index * 3 ] = face.mIndices[ 0 ];
		indices[ index * 3 + 1 ] = face.mIndices[ 1 ];
		indices[ index * 3 + 2 ] = face.mIndices[ 2 ];
	}

	memcpy( staticMesh->position.data(), mesh->mVertices, mesh->mNumVertices * sizeof( aiVector3D ) );

	return staticMesh;
}

std::shared_ptr<StaticMesh> AssetLoader::LoadStaticMesh( const std::string& fileName )
{
	std::vector<std::string> splits;
	std::stringstream ss( fileName );
	std::string buffer;

	while( std::getline( ss, buffer, '.' ) )
	{
		splits.push_back( buffer );
	}

	//const std::string& extension = splits.back();

	std::ifstream file( fileName );
	if( !file.is_open() )
	{
		return nullptr;
	}

	std::shared_ptr<StaticMesh> staticMesh;

	Assimp::Importer assimp;
	if( const aiScene* assimpScene = assimp.ReadFile( fileName, aiProcess_ConvertToLeftHanded | aiProcess_Triangulate | aiProcess_GenNormals ) )
	{
		staticMesh = LoadStaticMesh_Assimp( assimpScene );
	}

	file.close();

	return staticMesh;

	//std::vector<Vec3> position = {
	//	// Upper Left
	//	{ -1.0f, 1.0f, 0 },
	//	// Upper Right
	//	{ 1.0f, 1.0f, 0 },
	//	// Bottom right
	//	{ 1.0f, -1.0f, 0 },
	//	// Bottom left
	//	{ -1.0f, -1.0f, 0 }
	//};

	//std::vector<Vec2> uv = {
	//	// Upper Left
	//	{ 0, 0 },
	//	// Upper Right
	//	{ 1, 0 },
	//	// Bottom right
	//	{ 1, 1 },
	//	// Bottom left
	//	{ 0, 1 }
	//};

	//std::vector<uint32> indices = {
	//	0, 1, 2, 2, 3, 0
	//};
	//std::vector<std::vector<uint32>> meshes;
	//meshes.emplace_back( indices );

	//std::shared_ptr<StaticMesh> staticMesh( new StaticMesh() );
	//staticMesh->position = position;
	//staticMesh->uv = uv;
	//staticMesh->indices = meshes;

	//return staticMesh;
}
