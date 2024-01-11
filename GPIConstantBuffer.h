#pragma once

#include "Matrix.h"

__declspec( align( 256 ) )
struct ConstantBuffer
{
	Mat4x4 matViewProjection;
	Mat4x4 matViewProjectionInv;
	Mat4x4 matModel;
};