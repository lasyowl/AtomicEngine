#include "PBR.hlsl"

cbuffer PerFrameConstants : register (b0)
{
    float4x4 viewProjection;
    float4x4 viewProjectionInv;
    float3 viewPosition;
}

float2 VS_main(float3 position : POSITION) : TEXCOORD
{
    return position.xy;
}

struct PixelShaderOutput
{
    float4 rt0 : SV_TARGET0;
};

Texture2D gBufferA : register(t0);
Texture2D gBufferB : register(t1);
Texture2D depthStencil : register(t2);

SamplerState smp
{
    AddressU = Wrap;
    AddressV = Wrap;
};

PixelShaderOutput PS_main (float2 uv : TEXCOORD)
{
    float4 albedo = gBufferA.SampleLevel(smp, uv, 0);
    float3 outNormal = gBufferB.SampleLevel(smp, uv, 0).rgb * 2.0f - 1.0f;

    // Reconstruct world position from depth buffer
    float depth = depthStencil.SampleLevel(smp, uv, 0).r;
    
    // Convert UV to NDC (Normalized Device Coordinates)
    // Y coordinate is flipped because texture coordinates start at top-left
    float2 ndc = float2(uv.x * 2.0f - 1.0f, (1.0f - uv.y) * 2.0f - 1.0f);
    
    // Create clip space position
    float4 clipPos = float4(ndc, depth, 1.0f);
    
    // Transform from clip space to world space
    float4 worldPosPerspective = mul(clipPos, viewProjectionInv);
    float3 worldPosition = worldPosPerspective.xyz / worldPosPerspective.w;

    float3 color = PBR(albedo.rgb, float3(0, -1, 1), viewPosition, worldPosition, outNormal);
    
    PixelShaderOutput output;
    output.rt0 = float4(color, 1);

    return output;
}
