#include "PBR.hlsl"

cbuffer PerFrameConstants : register (b0)
{
    float4x4 viewProjection;
    float4x4 viewProjectionInv;
    float3 viewPosition;
}

cbuffer PerDrawConstants : register (b1)
{
    float4x4 model;
    float4x4 rotation;
}

struct VertexShaderOutput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
    float3x3 TBN : TBN;
};

float3x3 CalcTBN(float4x4 model, float3 normal, float3 tangent)
{
    float3 bitangent = cross(normal, tangent);
    
    float3 T = normalize(mul(float4(tangent, 0.0f), model).xyz);
    float3 B = normalize(mul(float4(bitangent, 0.0f), model).xyz);
    float3 N = normalize(mul(float4(normal, 0.0f), model).xyz);
    
    float sign = (dot(cross(N, T), B) < 0.0f) ? -1.0f : 1.0f;
    B = B * sign;
    
    return float3x3(T, B, N);
}

VertexShaderOutput VS_main(
    float3 position : POSITION,
    float3 normal : NORMAL,
    float2 uv : TEXCOORD,
    float3 tangent : TANGENT,
    uint vertexId : SV_VertexID)
{
    VertexShaderOutput output;
    
    output.position = mul(mul(float4(position, 1.0f), model), viewProjection);
    //output.normal = mul(float4(normal, 1.0f), rotation).xyz;
    output.uv = uv;
    
    output.TBN = CalcTBN(model, normal, tangent);
    
    return output;
}

struct PixelShaderOutput
{
    float4 rt0 : SV_TARGET0;
    float4 rt1 : SV_TARGET1;
};

Texture2D albedoTexture : register(t0);
Texture2D normalTexture : register(t1);

SamplerState smp
{
    AddressU = Wrap;
    AddressV = Wrap;
};

PixelShaderOutput PS_main (
    float4 position : SV_POSITION,
    float2 uv : TEXCOORD,
    float3x3 TBN : TBN)
{
    float4 albedo = albedoTexture.SampleLevel(smp, uv, 0);
    float opacity = albedo.a;
    
    // Discard pixels with low opacity
    clip(opacity - 0.5f);
    
    float3 outNormal = normalTexture.SampleLevel(smp, uv, 0).xyz * 2.0f - 1.0f;
    outNormal = normalize(mul(outNormal, TBN)) * 0.5f + 0.5f;
    
    PixelShaderOutput output;
    output.rt0 = albedo;
    output.rt1 = float4(outNormal, 1);

    return output;
}
