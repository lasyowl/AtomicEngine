#include "SampleMesh.h"
#include <Core/IntVector.h>

namespace SampleMesh
{
StaticMeshData GetQuad()
{
	StaticMeshData quad;
	quad.position = { { -1.0f, 1.0f, 0 }, { 1.0f, 1.0f, 0 }, { 1.0f, -1.0f, 0 }, { -1.0f, -1.0f, 0 } };
	quad.normal = { { 0, 0, -1 }, { 0, 0, -1 }, { 0, 0, -1 }, { 0, 0, -1 } };
	quad.uv = { { 0, 0 }, { 1, 0 }, { 1, 1 }, { 0, 1 } };
	quad.indices = { { { 0, 1, 2 }, { 2, 3, 0 } } };

	return quad;
}

StaticMeshData GetCube()
{
	StaticMeshData cube;
	cube.position = { 
		{ -1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f, -1.0f }, { -1.0f, 1.0f, -1.0f },
		{ -1.0f, 1.0f, -1.0f }, { 1.0f, 1.0f, -1.0f }, { 1.0f, -1.0f, -1.0f }, { -1.0f, -1.0f, -1.0f },
		{ -1.0f, 1.0f, 1.0f }, { -1.0f, 1.0f, -1.0f }, { -1.0f, -1.0f, -1.0f }, { -1.0f, -1.0f, 1.0f },
		{ 1.0f, 1.0f, 1.0f }, { -1.0f, 1.0f, 1.0f }, { -1.0f, -1.0f, 1.0f }, { 1.0f, -1.0f, 1.0f },
		{ 1.0f, 1.0f, -1.0f }, { 1.0f, 1.0f, 1.0f }, { 1.0f, -1.0f, 1.0f }, { 1.0f, -1.0f, -1.0f },
		{ -1.0f, -1.0f, -1.0f }, { 1.0f, -1.0f, -1.0f }, { 1.0f, -1.0f, 1.0f }, { -1.0f, -1.0f, 1.0f }
	};
	cube.normal = { 
		{ 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f },
		{ 0.0f, 0.0f, -1.0f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 0.0f, -1.0f },
		{ -1.0f, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f },
		{ 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f },
		{ 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f },
		{ 0.0f, -1.0f, 0.0f }, { 0.0f, -1.0f, 0.0f }, { 0.0f, -1.0f, 0.0f }, { 0.0f, -1.0f, 0.0f }
	};
	cube.uv = { { 0, 0 }, { 1, 0 }, { 1, 1 }, { 0, 1 }, { 0, 0 }, { 1, 0 }, { 1, 1 }, { 0, 1 } }; // invalid
	cube.indices = { {
		{ 0, 1, 2 }, { 2, 3, 0 },
		{ 4, 5, 6 }, { 6, 7, 4 },
		{ 8, 9, 10 }, { 10, 11, 8 },
		{ 12, 13, 14 }, { 14, 15, 12 },
		{ 16, 17, 18 }, { 18, 19, 16 },
		{ 20, 21, 22 }, { 22, 23, 20 } }
	};

	return cube;
}

StaticMeshData GetSphere()
{
	const Vec2 magicNumber = Vec2( 1.0f, 1.0f / ( 0.5f + 0.5f * sqrtf( 5.0 ) ) ).Normalize();
	const float mn0 = magicNumber.x;
	const float mn1 = magicNumber.y;

	std::vector<Vec3> vertices = {
		{ 0, mn1, -mn0 }, { mn1, mn0, 0 }, { -mn1, mn0, 0 }, { 0, mn1, mn0 },
		{ 0, -mn1, mn0 }, { -mn0, 0, mn1 }, { 0, -mn1, -mn0 }, { mn0, 0, -mn1 },
		{ mn0, 0, mn1 }, { -mn0, 0, -mn1 }, { mn1, -mn0, 0 }, { -mn1, -mn0, 0 }
	};

	std::vector<IVec3> triangles = {
		{ 2, 1, 0 }, { 1, 2, 3 }, { 5, 4, 3 }, { 4, 8, 3 },
		{ 7, 6, 0 }, { 6, 9, 0 }, { 11, 10, 4 }, { 10, 11, 6 },
		{ 9, 5, 2 }, { 5, 9, 11 }, { 8, 7, 1 }, { 7, 8, 10 },
		{ 2, 5, 3 }, { 8, 1, 3 }, { 9, 2, 0 }, { 1, 7, 0 },
		{ 11, 9, 6 }, { 7, 10, 6 }, { 5, 11, 4 }, { 10, 8, 4 }
	};

	std::map<std::pair<uint32, uint32>, uint32> splits;
	std::vector<IVec3> trianglesNew;

	for( uint32 index = 0; index < triangles.size(); ++index )
	{
		const IVec3& triangle = triangles[ index ];
		const Vec3 vertex0 = vertices[ triangle.x ];
		const Vec3 vertex1 = vertices[ triangle.y ];
		const Vec3 vertex2 = vertices[ triangle.z ];

		std::pair<uint32, uint32> splitKey0 = std::pair<uint32, uint32>( triangle.x, triangle.y );
		std::pair<uint32, uint32> splitKey1 = std::pair<uint32, uint32>( triangle.y, triangle.z );
		std::pair<uint32, uint32> splitKey2 = std::pair<uint32, uint32>( triangle.z, triangle.x );

		if( !splits.contains( splitKey0 ) )
		{
			const uint32 splitIndex = vertices.size();
			const Vec3 split = Vec3::Midpoint( vertex0, vertex1 ).Normalize();
			vertices.push_back( split );
			splits[ splitKey0 ] = splitIndex;
		}
		if( !splits.contains( splitKey1 ) )
		{
			const uint32 splitIndex = vertices.size();
			const Vec3 split = Vec3::Midpoint( vertex1, vertex2 ).Normalize();
			vertices.push_back( split );
			splits[ splitKey1 ] = splitIndex;
		}
		if( !splits.contains( splitKey2 ) )
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

		trianglesNew.push_back( IVec3( newIndex0, newIndex1, newIndex2 ) );
		trianglesNew.push_back( IVec3( newIndex1, newIndex4, newIndex2 ) );
		trianglesNew.push_back( IVec3( newIndex1, newIndex3, newIndex4 ) );
		trianglesNew.push_back( IVec3( newIndex2, newIndex4, newIndex5 ) );
	}

	StaticMeshData sphere;
	sphere.position = vertices;
	sphere.normal = sphere.position;
	sphere.uv = { { 0, 0 }, { 1, 0 }, { 1, 1 }, { 0, 1 }, { 0, 0 }, { 1, 0 }, { 1, 1 }, { 0, 1 } }; // invalid
	sphere.indices = { trianglesNew };

	return sphere;
}
}