#pragma once

#include <GPI/GPIResource.h>
#include <Core/Vector.h>

struct PBRMaterialConstant
{
	Vec3 baseColor;
	Vec3 normal;
	float roughness;
	float metalness;
};

struct PBRMaterial
{
	PBRMaterialConstant constants;

	IGPIResourceRef baseColor;
	IGPIResourceRef normal;
	IGPIResourceRef roughness;
	IGPIResourceRef metalness;
};