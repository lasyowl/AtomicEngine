#include "stdafx.h"
#include "RenderObject.h"

void CRenderObject::InitRenderResource()
{
	/* todo : Read from files */
	struct Vertex
	{
		float position[ 3 ];
		float uv[ 2 ];
	};

	Vertex vertices[ 4 ] = {
		// Upper Left
		{ { -1.0f, 1.0f, 0 },{ 0, 0 } },
		// Upper Right
		{ { 1.0f, 1.0f, 0 },{ 1, 0 } },
		// Bottom right
		{ { 1.0f, -1.0f, 0 },{ 1, 1 } },
		// Bottom left
		{ { -1.0f, -1.0f, 0 },{ 0, 1 } }
	};

	int indices[ 6 ] = {
		0, 1, 2, 2, 3, 0
	};

	VertexBuffer.InitBuffer( vertices, sizeof( vertices ) / 4, sizeof( vertices ) );
	IndexBuffer.InitBuffer( indices, sizeof( indices ) );
}
