#include "stdafx.h"
#include "SampleMesh.h"

namespace SampleMesh
{
StaticMesh GetQuad()
{
	StaticMesh quad;
	quad.position = { { -1.0f, 1.0f, 0 }, { 1.0f, 1.0f, 0 }, { 1.0f, -1.0f, 0 }, { -1.0f, -1.0f, 0 } };
	quad.normal = { { 0, 0, -1 }, { 0, 0, -1 }, { 0, 0, -1 }, { 0, 0, -1 } };
	quad.uv = { { 0, 0 }, { 1, 0 }, { 1, 1 }, { 0, 1 } };
	quad.indices = { { 0, 1, 2, 2, 3, 0 } };

	return quad;
}

//StaticMesh GetCube()
//{
//	StaticMesh cube;
//	cube.position = { { -1.0f, 1.0f, 0 }, { 1.0f, 1.0f, 0 }, { 1.0f, -1.0f, 0 }, { -1.0f, -1.0f, 0 } };
//	cube.uv = { { 0, 0 }, { 1, 0 }, { 1, 1 }, { 0, 1 } };
//	cube.indices = { { 0, 1, 2, 2, 3, 0 } };
//
//	return cube;
//}
}