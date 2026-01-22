#pragma once

#include <RenderBackend/RBShader.h>
#include <RenderBackend/RBResource.h>

class IRenderBackend;

class ShaderBase
{
protected:
    ShaderBase( RBShaderDesc desc )
    {
        _desc = desc;
    }

public:
    void CreateRenderResources( IRenderBackend& rb );

    uint32 GetParameterCount() const { return _desc.parameters.size(); }

    //@TODO: make private
public:
    // std::wstring FileName;
    // std::string bytecode;

    RBShaderDesc _desc;

    IRBShaderPtr _resource;
};

class GeometryVS : public ShaderBase
{
public:
    enum RootParamIndex : uint8
    {
        RPI_PerFrame,
        RPI_PerDraw,
    };

public:
    GeometryVS()
        : ShaderBase( RBShaderDesc
                   {
                        .type = EShaderType::VertexShader,
                        .file = "../Shader/GeometryPass.hlsl",
                        .entry = "VS_main",
                        .parameters =
                        {
                            { "PerFrame", EShaderParameterType::ConstantBuffer, RPI_PerFrame, 0 },
                            { "PerDraw", EShaderParameterType::ConstantBuffer, RPI_PerDraw, 0 }
                        }
                    } )
    {}
};

class GeometryPS : public ShaderBase
{
public:
    enum RootParamIndex : uint8
    {
        RPI_TexBegin,
        RPI_TexAlbedo = RPI_TexBegin,
        RPI_TexNormal,
        RPI_TexEnd = RPI_TexNormal,
        RPI_TexCount = RPI_TexEnd + 1,
    };

public:
    GeometryPS()
        : ShaderBase( RBShaderDesc
                   {
                        .type = EShaderType::PixelShader,
                        .file = "../Shader/GeometryPass.hlsl",
                        .entry = "PS_main",
                        .parameters =
                        {
                            { "testTex", EShaderParameterType::DescriptorTable, RPI_TexBegin, 0, RPI_TexCount }
                        }
                    } )
    {}
};

class FullScreenQuadVS : public ShaderBase
{
public:
    enum RootParamIndex : uint8
    {
        RPI_PerFrame,
        RPI_PerDraw,
    };

public:
    FullScreenQuadVS()
        : ShaderBase( RBShaderDesc
                   {
                        .type = EShaderType::VertexShader,
                        .file = "../Shader/LightingPass.hlsl",
                        .entry = "VS_main",
                        .parameters =
                        {
                            { "PerFrame", EShaderParameterType::ConstantBuffer, RPI_PerFrame, 0 },
                            { "PerDraw", EShaderParameterType::ConstantBuffer, RPI_PerDraw, 0 }
                        }
                    } )
    {}
};

class LightingPS : public ShaderBase
{
public:
    enum RootParamIndex : uint8
    {
        RPI_TexBegin,
        RPI_TexAlbedo = RPI_TexBegin,
        RPI_TexNormal,
        RPI_TexEnd = RPI_TexNormal,
        RPI_TexCount = RPI_TexEnd + 1,
    };

public:
    LightingPS()
        : ShaderBase( RBShaderDesc
                   {
                        .type = EShaderType::PixelShader,
                        .file = "../Shader/LightingPass.hlsl",
                        .entry = "PS_main",
                        .parameters =
                        {
                            { "testTex", EShaderParameterType::DescriptorTable, RPI_TexBegin, 0, RPI_TexCount }
                        }
                    } )
    {}
};
