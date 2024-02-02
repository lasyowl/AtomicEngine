#include "AssetLoader.h"
#include <Core/IntVector.h>

#include <ThirdParty/assimp/Importer.hpp>
#include <ThirdParty/assimp/scene.h>
#include <ThirdParty/assimp/postprocess.h>
#include <ThirdParty/FreeImage/x64/FreeImage.h>

#include <sstream>
#include <fstream>

#include "StaticMesh.h"
#include "RawImage.h"

std::shared_ptr<StaticMeshDataGroup> LoadStaticMeshData_Assimp( const aiScene* scene )
{
	std::shared_ptr<StaticMeshDataGroup> dataGroup = std::make_shared<StaticMeshDataGroup>();
	dataGroup->datas.resize( scene->mNumMeshes );

	for( uint32 index = 0; index < scene->mNumMeshes; ++index )
	{
		StaticMeshData& data = dataGroup->datas[ index ];
		const aiMesh* mesh = scene->mMeshes[ index ];

		data.name = mesh->mName.C_Str();
		data.position.resize( mesh->mNumVertices );
		data.normal.resize( mesh->mNumVertices );
		data.uv.resize( mesh->mNumVertices );

		data.indices.resize( 1 );
		std::vector<IVec3>& indices = data.indices[ 0 ];
		indices.resize( mesh->mNumFaces );

		for( int32 index = 0; index < mesh->mNumFaces; ++index )
		{
			const aiFace& face = mesh->mFaces[ index ];

			indices[ index ].x = face.mIndices[ 0 ];
			indices[ index ].y = face.mIndices[ 1 ];
			indices[ index ].z = face.mIndices[ 2 ];
		}

		memcpy( data.position.data(), mesh->mVertices, mesh->mNumVertices * sizeof( aiVector3D ) );
		if( mesh->mNormals )
		{
			memcpy( data.normal.data(), mesh->mNormals, mesh->mNumVertices * sizeof( aiVector3D ) );
		}
		if( mesh->mTextureCoords[ 0 ] )
		{
			const aiVector3D* uvs = mesh->mTextureCoords[ 0 ];

			for( int32 index = 0; index < mesh->mNumVertices; ++index )
			{
				const aiVector3D& uv = uvs[ index ];

				data.uv[ index ].x = uv.x;
				data.uv[ index ].y = uv.y;
			}
		}
	}

	return dataGroup;
}

std::shared_ptr<StaticMeshDataGroup> AssetLoader::LoadStaticMeshData( const std::string& fileName )
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

	std::shared_ptr<StaticMeshDataGroup> dataGroup;

	Assimp::Importer assimp;
	if( const aiScene* assimpScene = assimp.ReadFile( fileName, aiProcess_ConvertToLeftHanded | aiProcess_Triangulate | aiProcess_GenUVCoords | aiProcess_GenSmoothNormals ) )
	{
		dataGroup = LoadStaticMeshData_Assimp( assimpScene );
	}

	file.close();

	return dataGroup;
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