cbuffer PerFrameConstants : register (b0)
{
    float4x4 viewProjection;
}

Texture2D gDiffuse : register (t0);
Texture2D gNormal : register (t1);
Texture2D gBuffer2 : register (t2);
Texture2D gDepth : register (t3);
SamplerState smp { AddressU = Wrap; AddressV = Wrap; };

struct VertexShaderOutput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

VertexShaderOutput VS_main(
    float3 position : POSITION,
    float2 uv : TEXCOORD)
{
    VertexShaderOutput output;

    output.position = float4(position, 1.0f);
    output.uv = uv;

    return output;
}

float4 PS_main (
    float4 position : SV_POSITION,
    float2 uv : TEXCOORD) : SV_TARGET
{
    if(gNormal.Sample(smp, uv).r == 0) discard;

    return float4(gDepth.Sample(smp, uv).xxx, 1);
    
    float3 screenNormal = float3(0, 0, -1);
    float3 pixelNormal = 2.0f * gNormal.Sample(smp, uv).xyz - 1.0f;
    float intensity = dot(screenNormal, pixelNormal);
    float3 diffuse = intensity * float3(1, 1, 1);

    return float4(diffuse, 1);
}