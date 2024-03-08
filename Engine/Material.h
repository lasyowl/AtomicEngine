#pragma once

#include <RHI/RHIResource.h>
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

	IRHIResourceRef baseColor;
	IRHIResourceRef normal;
	IRHIResourceRef roughness;
	IRHIResourceRef metalness;
};