#include "stdafx.h"
#include "AssetLoader.h"
#include "IntVector.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <FreeImage/x64/FreeImage.h>

std::shared_ptr<StaticMeshData> LoadStaticMeshData_WavefrontObj( std::ifstream& file )
{
	std::shared_ptr<StaticMeshData> staticMeshData = std::make_shared<StaticMeshData>();

	//std::vector<Vec3>& position = staticMesh->position;
	//std::vector<Vec2>& uv = staticMesh->uv;
	//std::vector<std::vector<IVec3>>& index = staticMesh->indices;
	//std::vector<IVec3>& currentIndex = index.emplace_back();

	//std::string line;
	//while( std::getline( file, line ) )
	//{
	//	std::stringstream stream( line );
	//	std::string header;
	//	stream >> header;

	//	constexpr uint32 NUM_VERTEX = 3;
	//	constexpr uint32 NUM_UV = 2;

	//	if( header.compare( "o" ) == 0 )
	//	{
	//		if( !staticMesh->name.empty() ) break; // temp
	//		stream >> staticMesh->name;
	//	}
	//	else if( header.compare( "v" ) == 0 )
	//	{
	//		float buffer[ 3 ];
	//		for( uint32 iter = 0; iter < NUM_VERTEX; ++iter )
	//		{
	//			stream >> buffer[ iter ];
	//		}
	//		Vec3 buffer1 = { buffer[ 0 ], buffer[ 1 ], buffer[ 2 ] };
	//		position.emplace_back( buffer1 );
	//	}
	//	else if( header.compare( "vt" ) == 0 )
	//	{
	//		float buffer[ 2 ];
	//		for( uint32 iter = 0; iter < NUM_UV; ++iter )
	//		{
	//			stream >> buffer[ iter ];
	//		}
	//		Vec2 buffer1 = { buffer[ 0 ], buffer[ 1 ] };
	//		uv.emplace_back( buffer1 );
	//	}
	//	else if( header.compare( "vn" ) == 0 )
	//	{
	//	}
	//	else if( header.compare( "g" ) == 0 )
	//	{

	//	}
	//	else if( header.compare( "f" ) == 0 )
	//	{
	//		for( uint32 iter = 0; iter < NUM_VERTEX; ++iter )
	//		{
	//			std::string lineBuffer;
	//			stream >> lineBuffer;

	//			uint32 vertexIndex = 0;
	//			uint32 uvIndex = 0;

	//			std::stringstream vertexStream( lineBuffer );
	//			std::string buffer;
	//			if( std::getline( vertexStream, buffer, '/' ) )
	//			{
	//				vertexIndex = std::stoi( buffer ) - 1;
	//				uvIndex = vertexIndex;
	//			}
	//			if( std::getline( vertexStream, buffer, '/' ) )
	//			{
	//				uvIndex = std::stoi( buffer ) - 1;
	//			}

	//			uint32 indexMax = max( vertexIndex, uvIndex );
	//			if( position.size() <= indexMax )
	//			{
	//				position.resize( indexMax + 1 );
	//			}

	//			if( vertexIndex != uvIndex )
	//			{
	//				position[ uvIndex ] = position[ vertexIndex ];
	//			}

	//			currentIndex.emplace_back( uvIndex );
	//		}
	//	}
	//}

	return staticMeshData;
}

std::shared_ptr<StaticMeshData> LoadStaticMeshData_Assimp( const aiScene* scene )
{
	std::shared_ptr<StaticMeshData> staticMeshData = std::make_shared<StaticMeshData>();

	const aiMesh* mesh = scene->mMeshes[ 0 ];

	staticMeshData->position.resize( mesh->mNumVertices );
	staticMeshData->normal.resize( mesh->mNumVertices );
	//staticMeshData->uv.resize( mesh->mNumVertices );
	staticMeshData->indices.resize( 1 );
	std::vector<IVec3>& indices = staticMeshData->indices[ 0 ];
	indices.resize( mesh->mNumFaces );

	for( int32 index = 0; index < mesh->mNumFaces; ++index )
	{
		const aiFace& face = mesh->mFaces[ index ];

		indices[ index ].x = face.mIndices[ 0 ];
		indices[ index ].y = face.mIndices[ 1 ];
		indices[ index ].z = face.mIndices[ 2 ];
	}

	memcpy( staticMeshData->position.data(), mesh->mVertices, mesh->mNumVertices * sizeof( aiVector3D ) );
	if( mesh->mNormals )
	{
		memcpy( staticMeshData->normal.data(), mesh->mNormals, mesh->mNumVertices * sizeof( aiVector3D ) );
	}

	return staticMeshData;
}

std::shared_ptr<StaticMeshData> AssetLoader::LoadStaticMeshData( const std::string& fileName )
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

	std::shared_ptr<StaticMeshData> staticMeshData;

	Assimp::Importer assimp;
	if( const aiScene* assimpScene = assimp.ReadFile( fileName, aiProcess_ConvertToLeftHanded | aiProcess_Triangulate | aiProcess_GenUVCoords | aiProcess_GenSmoothNormals ) )
	{
		staticMeshData = LoadStaticMeshData_Assimp( assimpScene );
	}

	file.close();

	return staticMeshData;
}

std::shared_ptr<RawImage> AssetLoader::LoadRawImage( const std::string& fileName )
{
	static bool bFreeImageInitialized = false;
	if( !bFreeImageInitialized )
	{
		FreeImage_Initialise();
	}

	FIBITMAP* image = FreeImage_Load( FIF_PNG, fileName.c_str(), PNG_DEFAULT );
	if( !image )
	{
		return nullptr;
	}

	FreeImage_FlipVertical( image );
	image = FreeImage_ConvertTo32Bits( image );

	BITMAPINFO* info = FreeImage_GetInfo( image );

	std::shared_ptr<RawImage> result( new RawImage() );
	result->width = info->bmiHeader.biWidth;
	result->height = info->bmiHeader.biHeight;

	uint32 imageSize = result->width * result->height * 4;
	result->data.resize( imageSize );
	memcpy( result->data.data(), FreeImage_GetBits( image ), imageSize );
	
	FreeImage_Unload( image );

	return result;
}