#pragma once

#include <GPI/GPIResource.h>
#include <Core/Vector.h>

struct PBRMaterialSimple
{
	Vec3 baseColor;
	Vec3 normal;
	float roughness;
	float metalness;
};

struct PBRMaterial
{
	IGPIResourceRef baseColor;
	IGPIResourceRef normal;
	IGPIResourceRef roughness;
	IGPIResourceRef metalness;
};