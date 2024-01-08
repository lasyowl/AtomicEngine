
cbuffer PerFrameConstants : register (b0)
{
    float4x4 viewProjection;
}

Texture2D GBuffer0 : register (t0);
Texture2D GBuffer1 : register (t1);
Texture2D GBuffer2 : register (t2);
Texture2D GBuffer3 : register (t3);
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
    float4 color = GBuffer1.Sample(smp, uv);

    return float4(color.rgb, 1);
}