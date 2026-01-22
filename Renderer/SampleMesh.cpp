#include "SampleMesh.h"
#include <Core/IntVector.h>

namespace SampleMesh
{
    MeshData GetQuad()
    {
        MeshData data;
        data.name = "quad";
        /*data.vertexStride = sizeof( Vec3 ) + sizeof( Vec3 ) + sizeof( Vec2 );
        data.position = { { -1.0f, 1.0f, 0 }, { 1.0f, 1.0f, 0 }, { 1.0f, -1.0f, 0 }, { -1.0f, -1.0f, 0 } };
        data.normal = { { 0, 0, -1 }, { 0, 0, -1 }, { 0, 0, -1 }, { 0, 0, -1 } };
        data.uv = { { 0, 0 }, { 1, 0 }, { 1, 1 }, { 0, 1 } };
        data.indices = { { { 0, 1, 2 }, { 2, 3, 0 } } };*/

        return data;
    }

    MeshData GetPlane()
    {
        MeshData data;
        data.name = "plane";
        /*data.position = { { -1, 0, 1 }, { 1, 0, 1 }, { 1, 0, -1 }, { -1, 0, -1 } };
        data.normal = { { 0, 1, 0 }, { 0, 1, 0 }, { 0, 1, 0 }, { 0, 1, 0 } };
        data.uv = { { 0, 0 }, { 1, 0 }, { 1, 1 }, { 0, 1 } };
        data.indices = { { { 0, 1, 2 }, { 2, 3, 0 } } };*/

        return data;
    }

    MeshData GetCube()
    {
        MeshData data;
        /*data.name = "cube";
        data.position = {
            { -1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f, -1.0f }, { -1.0f, 1.0f, -1.0f },
            { -1.0f, 1.0f, -1.0f }, { 1.0f, 1.0f, -1.0f }, { 1.0f, -1.0f, -1.0f }, { -1.0f, -1.0f, -1.0f },
            { -1.0f, 1.0f, 1.0f }, { -1.0f, 1.0f, -1.0f }, { -1.0f, -1.0f, -1.0f }, { -1.0f, -1.0f, 1.0f },
            { 1.0f, 1.0f, 1.0f }, { -1.0f, 1.0f, 1.0f }, { -1.0f, -1.0f, 1.0f }, { 1.0f, -1.0f, 1.0f },
            { 1.0f, 1.0f, -1.0f }, { 1.0f, 1.0f, 1.0f }, { 1.0f, -1.0f, 1.0f }, { 1.0f, -1.0f, -1.0f },
            { -1.0f, -1.0f, -1.0f }, { 1.0f, -1.0f, -1.0f }, { 1.0f, -1.0f, 1.0f }, { -1.0f, -1.0f, 1.0f }
        };
        data.normal = {
            { 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f },
            { 0.0f, 0.0f, -1.0f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 0.0f, -1.0f },
            { -1.0f, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f },
            { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f },
            { 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f },
            { 0.0f, -1.0f, 0.0f }, { 0.0f, -1.0f, 0.0f }, { 0.0f, -1.0f, 0.0f }, { 0.0f, -1.0f, 0.0f }
        };
        data.uv = {
            { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f },
            { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f },
            { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f },
            { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f },
            { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f },
            { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f }
        };
        data.indices = { {
            { 0, 1, 2 }, { 2, 3, 0 },
            { 4, 5, 6 }, { 6, 7, 4 },
            { 8, 9, 10 }, { 10, 11, 8 },
            { 12, 13, 14 }, { 14, 15, 12 },
            { 16, 17, 18 }, { 18, 19, 16 },
            { 20, 21, 22 }, { 22, 23, 20 } }
        };*/

        return data;
    }

    MeshData GetSphere()
    {
        const Vec2 magicNumber = Vec2( 1.0f, 1.0f / ( 0.5f + 0.5f * sqrtf( 5.0 ) ) ).Normalize();
        const float mn0 = magicNumber.x;
        const float mn1 = magicNumber.y;

        std::vector<Vec3> vertices = {
            { 0, mn1, -mn0 }, { mn1, mn0, 0 }, { -mn1, mn0, 0 }, { 0, mn1, mn0 },
            { 0, -mn1, mn0 }, { -mn0, 0, mn1 }, { 0, -mn1, -mn0 }, { mn0, 0, -mn1 },
            { mn0, 0, mn1 }, { -mn0, 0, -mn1 }, { mn1, -mn0, 0 }, { -mn1, -mn0, 0 }
        };

        std::vector<IVec3> indices = {
            { 2, 1, 0 }, { 1, 2, 3 }, { 5, 4, 3 }, { 4, 8, 3 },
            { 7, 6, 0 }, { 6, 9, 0 }, { 11, 10, 4 }, { 10, 11, 6 },
            { 9, 5, 2 }, { 5, 9, 11 }, { 8, 7, 1 }, { 7, 8, 10 },
            { 2, 5, 3 }, { 8, 1, 3 }, { 9, 2, 0 }, { 1, 7, 0 },
            { 11, 9, 6 }, { 7, 10, 6 }, { 5, 11, 4 }, { 10, 8, 4 }
        };

        auto _tesselate = [ &vertices, &indices ]()
        {
            std::map<std::pair<uint32, uint32>, uint32> splits;
            std::vector<IVec3> indicesNew;

            for ( uint32 index = 0; index < indices.size(); ++index )
            {
                const IVec3& triangle = indices[ index ];
                const Vec3 vertex0 = vertices[ triangle.x ];
                const Vec3 vertex1 = vertices[ triangle.y ];
                const Vec3 vertex2 = vertices[ triangle.z ];

                std::pair<uint32, uint32> splitKey0 = std::pair<uint32, uint32>( triangle.x, triangle.y );
                std::pair<uint32, uint32> splitKey1 = std::pair<uint32, uint32>( triangle.y, triangle.z );
                std::pair<uint32, uint32> splitKey2 = std::pair<uint32, uint32>( triangle.z, triangle.x );

                if ( !splits.contains( splitKey0 ) )
                {
                    const uint32 splitIndex = vertices.size();
                    const Vec3 split = Vec3::Midpoint( vertex0, vertex1 ).Normalize();
                    vertices.push_back( split );
                    splits[ splitKey0 ] = splitIndex;
                }
                if ( !splits.contains( splitKey1 ) )
                {
                    const uint32 splitIndex = vertices.size();
                    const Vec3 split = Vec3::Midpoint( vertex1, vertex2 ).Normalize();
                    vertices.push_back( split );
                    splits[ splitKey1 ] = splitIndex;
                }
                if ( !splits.contains( splitKey2 ) )
                {
                    const uint32 splitIndex = vertices.size();
                    const Vec3 split = Vec3::Midpoint( vertex2, vertex0 ).Normalize();
                    vertices.push_back( split );
                    splits[ splitKey2 ] = splitIndex;
                }

                uint32 newIndex0 = triangle.x;
                uint32 newIndex1 = splits[ splitKey0 ];
                uint32 newIndex2 = splits[ splitKey2 ];
                uint32 newIndex3 = triangle.y;
                uint32 newIndex4 = splits[ splitKey1 ];
                uint32 newIndex5 = triangle.z;

                indicesNew.push_back( IVec3( newIndex0, newIndex1, newIndex2 ) );
                indicesNew.push_back( IVec3( newIndex1, newIndex4, newIndex2 ) );
                indicesNew.push_back( IVec3( newIndex1, newIndex3, newIndex4 ) );
                indicesNew.push_back( IVec3( newIndex2, newIndex4, newIndex5 ) );
            }

            std::swap( indices, indicesNew );
        };

        //_tesselate();
        //_tesselate();
        _tesselate();

        // compute per-vertex normals from triangle faces
        std::vector<Vec3> normals( vertices.size(), Vec3() );

        auto cross = []( const Vec3& a, const Vec3& b ) -> Vec3
        {
            return Vec3(
                a.y * b.z - a.z * b.y,
                a.z * b.x - a.x * b.z,
                a.x * b.y - a.y * b.x
            );
        };

        for ( const IVec3& tri : indices )
        {
            const Vec3& p0 = vertices[ tri.x ];
            const Vec3& p1 = vertices[ tri.y ];
            const Vec3& p2 = vertices[ tri.z ];

            Vec3 e1( p1.x - p0.x, p1.y - p0.y, p1.z - p0.z );
            Vec3 e2( p2.x - p0.x, p2.y - p0.y, p2.z - p0.z );

            Vec3 faceNormal = cross( e1, e2 );
            // don't normalize here to preserve area-weighted accumulation
            normals[ tri.x ] = normals[ tri.x ] + faceNormal;
            normals[ tri.y ] = normals[ tri.y ] + faceNormal;
            normals[ tri.z ] = normals[ tri.z ] + faceNormal;
        }

        for ( Vec3& n : normals )
        {
            // normalize accumulated normal, guard against zero-length
            float mag = n.Magnitude();
            if ( mag > 1e-6f )
            {
                n.Normalize();
            }
            else
            {
                n = Vec3( 0.0f, 1.0f, 0.0f ); // fallback
            }
        }

        std::vector<uint8> vertexData;
        vertexData.reserve( vertices.size() * ( sizeof( Vec3 ) + sizeof( Vec3 ) ) );
        for ( int32 index = 0; index < vertices.size(); ++index )
        {
            // position
            vertexData.insert( vertexData.end(), (uint8*)&vertices[ index ], (uint8*)&vertices[ index ] + sizeof( Vec3 ) );
            // normal (computed)
            vertexData.insert( vertexData.end(), (uint8*)&normals[ index ], (uint8*)&normals[ index ] + sizeof( Vec3 ) );
        }

        MeshData meshData;
        meshData.name = "sphere";
        meshData.attributeFlag = EMeshAttribute::MA_Position | EMeshAttribute::MA_Normal;
        meshData.positionOffset = 0;
        meshData.vertexStride = sizeof( Vec3 ) + sizeof( Vec3 );
        meshData.vertices = vertexData;

        //meshData.vertices.resize( vertices.size() * meshData.vertexStride );
        //memcpy( meshData.vertices.data(), vertices.data(), meshData.vertices.size() );
        //meshData.normal = meshData.position;
        //meshData.uv = { { 0, 0 }, { 1, 0 }, { 1, 1 }, { 0, 1 }, { 0, 0 }, { 1, 0 }, { 1, 1 }, { 0, 1 } }; // invalid
        meshData.indices = { indices };

        return meshData;
    }
}