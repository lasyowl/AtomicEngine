#pragma once

#include <RenderBackend/RBResource.h>
#include <Core/Vector.h>

class Texture;
class IRenderBackend;

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

    IRBResourcePtr baseColor;
    IRBResourcePtr normal;
    IRBResourcePtr roughness;
    IRBResourcePtr metalness;
};

class Material
{
public:
    void CreateRenderResources( IRenderBackend& rb );

public:
    std::string name;

    Vec3 albedo;
    float opacity;
    Vec3 normal;
    float roughness;
    float metallic;
    float ambientOcclusion;

    std::shared_ptr<Texture> albedoTexture;
    std::shared_ptr<Texture> opacityTexture;
    std::shared_ptr<Texture> normalTexture;
    std::shared_ptr<Texture> roughnessTexture;
    std::shared_ptr<Texture> metallicTexture;
    std::shared_ptr<Texture> ambientOcclusionTexture;

    IRBTextureViewTablePtr textureViewTable;

    //std::string shaderCode;
    //IRBTextureViewTableRef textureResources;

    //std::vector<Texture> textures;
};
using MaterialPtr = std::shared_ptr<Material>;
