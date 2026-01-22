#include "AssetLoader.h"

#include <Core/IntVector.h>
#include <Core/Cache.h>

#include <ThirdParty/assimp/Importer.hpp>
#include <ThirdParty/assimp/scene.h>
#include <ThirdParty/assimp/postprocess.h>
#include <ThirdParty/FreeImage/x64/FreeImage.h>

#include <sstream>
#include <fstream>

#include <Renderer/Mesh.h>
#include <Renderer/Texture.h>
#include <Renderer/Material.h>

std::vector<std::shared_ptr<MeshData>> LoadMeshData_Assimp( const aiScene* scene )
{
    std::vector<std::shared_ptr<MeshData>> meshArray;

    for ( uint32 index = 0; index < scene->mNumMeshes; ++index )
    {
        const aiMesh* mesh = scene->mMeshes[ index ];

        std::shared_ptr<MeshData> meshData = meshArray.emplace_back( std::make_shared<MeshData>() );
        meshData->attributeFlag = MA_Position | MA_Normal | MA_UV | MA_Tangent;
        meshData->name = mesh->mName.C_Str();

        if ( meshData->attributeFlag & MA_Position )
        {
            meshData->positionOffset = meshData->vertexStride;
            meshData->vertexStride += sizeof( Vec3 );
        }
        if ( meshData->attributeFlag & MA_Normal )
        {
            meshData->normalOffset = meshData->vertexStride;
            meshData->vertexStride += sizeof( Vec3 );
        }
        if ( meshData->attributeFlag & MA_UV )
        {
            meshData->uvOffset = meshData->vertexStride;
            meshData->vertexStride += sizeof( Vec2 );
        }
        if ( meshData->attributeFlag & MA_Tangent )
        {
            meshData->vertexStride += sizeof( Vec3 );
        }

        const uint64 vertexByteSize = meshData->vertexStride * mesh->mNumVertices;
        meshData->vertices.resize( vertexByteSize );

        uint8* vertexPtr = meshData->vertices.data();
        for ( int32 index = 0; index < mesh->mNumVertices; ++index )
        {
            if ( meshData->attributeFlag & MA_Position )
            {
                memcpy( vertexPtr, &mesh->mVertices[ index ], sizeof( Vec3 ) );
                vertexPtr += sizeof( Vec3 );
            }
            if ( meshData->attributeFlag & MA_Normal )
            {
                memcpy( vertexPtr, &mesh->mNormals[ index ], sizeof( Vec3 ) );
                vertexPtr += sizeof( Vec3 );
            }
            if ( meshData->attributeFlag & MA_UV )
            {
                memcpy( vertexPtr, &mesh->mTextureCoords[ 0 ][ index ], sizeof( Vec2 ) );
                vertexPtr += sizeof( Vec2 );
            }
            if ( meshData->attributeFlag & MA_Tangent )
            {
                memcpy( vertexPtr, &mesh->mTangents[ index ], sizeof( Vec3 ) );
                vertexPtr += sizeof( Vec3 );
            }
        }

        meshData->indices.resize( 1 );
        std::vector<IVec3>& indices = meshData->indices[ 0 ];
        indices.resize( mesh->mNumFaces );

        for ( int32 index = 0; index < mesh->mNumFaces; ++index )
        {
            const aiFace& face = mesh->mFaces[ index ];

            indices[ index ].x = face.mIndices[ 0 ];
            indices[ index ].y = face.mIndices[ 1 ];
            indices[ index ].z = face.mIndices[ 2 ];
        }

        const aiMaterial* material = scene->mMaterials[ mesh->mMaterialIndex ];
        meshData->materialName = material->GetName().C_Str();
    }

    return meshArray;
}

void LoadMaterial_Assimp( const aiScene* scene, const std::string& root )
{
    for ( int32 index = 0; index < scene->mNumMaterials; ++index )
    {
        const aiMaterial* material = scene->mMaterials[ index ];

        std::string name = material->GetName().C_Str();
        MaterialPtr outMaterial = TCache<Material>::GetInstance().Add( name, std::make_shared<Material>() );

        outMaterial->name = name;
        material->Get( AI_MATKEY_COLOR_DIFFUSE, reinterpret_cast<aiColor3D&>( outMaterial->albedo ) );
        material->Get( AI_MATKEY_OPACITY, outMaterial->opacity );
        material->Get( AI_MATKEY_SHININESS, outMaterial->metallic );
        material->Get( AI_MATKEY_SHININESS, outMaterial->roughness );

        aiString relativePath;
        if ( material->GetTexture( aiTextureType_DIFFUSE, 0, &relativePath ) == AI_SUCCESS )
        {
            const std::string fullPath = root + "/" + std::string( relativePath.C_Str() );
            outMaterial->albedoTexture = AssetLoader::LoadTexture( fullPath );
        }
        if ( material->GetTexture( aiTextureType_OPACITY, 0, &relativePath ) == AI_SUCCESS )
        {
            const std::string fullPath = root + "/" + std::string( relativePath.C_Str() );
            outMaterial->opacityTexture = AssetLoader::LoadTexture( fullPath );
        }
        if ( material->GetTexture( aiTextureType_DISPLACEMENT, 0, &relativePath ) == AI_SUCCESS )
        {
            const std::string fullPath = root + "/" + std::string( relativePath.C_Str() );
            outMaterial->normalTexture = AssetLoader::LoadTexture( fullPath );
        }
        if ( material->GetTexture( aiTextureType_SPECULAR, 0, &relativePath ) == AI_SUCCESS )
        {
            const std::string fullPath = root + "/" + std::string( relativePath.C_Str() );
            outMaterial->metallicTexture = AssetLoader::LoadTexture( fullPath );
        }
        if ( material->GetTexture( aiTextureType_SHININESS, 0, &relativePath ) == AI_SUCCESS )
        {
            const std::string fullPath = root + "/" + std::string( relativePath.C_Str() );
            outMaterial->roughnessTexture = AssetLoader::LoadTexture( fullPath );
        }
    }
}

std::string get_directory_from_path( const std::string& filePath )
{
    const uint32 found = filePath.find_last_of( "/" );
    return filePath.substr( 0, found );
}

std::vector<std::shared_ptr<MeshData>> AssetLoader::LoadMeshData( const std::string& fileName )
{
    std::vector<std::string> splits;
    std::stringstream ss( fileName );
    std::string buffer;

    while ( std::getline( ss, buffer, '.' ) )
    {
        splits.push_back( buffer );
    }

    //const std::string& extension = splits.back();

    std::vector<std::shared_ptr<MeshData>> meshArray;

    std::ifstream file( fileName );
    if ( file.is_open() )
    {
        Assimp::Importer assimp;
        if ( const aiScene* assimpScene = assimp.ReadFile( fileName, aiProcess_ConvertToLeftHanded | aiProcess_Triangulate | aiProcess_GenUVCoords | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace ) )
        {
            LoadMaterial_Assimp( assimpScene, get_directory_from_path( fileName ) );
            meshArray = LoadMeshData_Assimp( assimpScene );
        }

        file.close();
    }

    return meshArray;
}

FREE_IMAGE_FORMAT translate_image_format( const std::string_view extension )
{
    if ( extension == "png" ) return FIF_PNG;
    else if ( extension == "tga" ) return FIF_TARGA;

    return FIF_UNKNOWN;
}

static std::string_view get_file_extension( std::string_view filename )
{
    const uint32 separatorIndex = filename.find_last_of( '.' );

    if ( separatorIndex == std::string_view::npos )
        return nullptr;

    return filename.substr( separatorIndex + 1 );
}

TexturePtr AssetLoader::LoadTexture( const std::string& fileName )
{
    static bool bFreeImageInitialized = false;
    if ( !bFreeImageInitialized )
    {
        FreeImage_Initialise();
    }

    const std::string_view extension = get_file_extension( fileName );
    const FREE_IMAGE_FORMAT imageFormat = translate_image_format( extension );

    FIBITMAP* image = FreeImage_Load( imageFormat, fileName.c_str(), PNG_DEFAULT );
    if ( !image )
    {
        return nullptr;
    }

    FreeImage_FlipVertical( image );
    image = FreeImage_ConvertTo32Bits( image );

    BITMAPINFO* info = FreeImage_GetInfo( image );

    const uint32 width = info->bmiHeader.biWidth;
    const uint32 height = info->bmiHeader.biHeight;
    const uint32 imageSize = width * height * 4;

    std::vector<uint8> data( imageSize );
    memcpy( data.data(), FreeImage_GetBits( image ), imageSize );

    FreeImage_Unload( image );

    return TCache<Texture>::GetInstance().FindOrAdd( fileName, fileName, data, width, height );
}

std::shared_ptr<Material> LoadMaterial( const std::string& fileName )
{
    std::vector<std::shared_ptr<Material>> materials;

    //std::ifstream file( fileName );
    //if ( file.is_open() )
    //{
    //    Assimp::Importer assimp;
    //    if ( const aiScene* assimpScene = assimp.ReadFile( fileName, 0 ) )
    //    {
    //        LoadMaterial_Assimp( assimpScene );
    //    }

    //    file.close();
    //}

    return nullptr;
}
