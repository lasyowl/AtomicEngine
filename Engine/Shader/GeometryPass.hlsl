#if D3D12_SAMPLE_CONSTANT_BUFFER
cbuffer PerFrameConstants : register (b0)
{
    float4x4 viewProjection;
    float4x4 viewProjectionInv;
}

cbuffer PerDrawConstants : register (b1)
{
    float4x4 model;
}

Texture2D testTex : register (t0);
SamplerState smp { AddressU = Wrap; AddressV = Wrap; };

struct VertexShaderOutput
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
};

VertexShaderOutput VS_main(
    float3 position : POSITION,
    float3 normal : NORMAL,
    float2 uv : TEXCOORD)
{
    VertexShaderOutput output;

    output.position = mul(mul(float4(position, 1.0f), model), viewProjection);
    output.normal = mul(float4(normal, 1.0f), model).xyz;
    output.uv = uv;

    return output;
}

struct PixelShaderOutput
{
    float4 rt0 : SV_TARGET0;
    float4 rt1 : SV_TARGET1;
    float4 rt2 : SV_TARGET2;
    float4 rt3 : SV_TARGET3;
};

PixelShaderOutput PS_main (
    float4 position : SV_POSITION,
    float3 normal : NORMAL,
    float2 uv : TEXCOORD)
{
    float4 color = testTex.Sample(smp, uv);

    PixelShaderOutput output;
    output.rt0 = float4(1, 1, 1, 1);
    output.rt1 = float4(normalize(normal) * 0.5f + 0.5f, 1);
    output.rt2 = float4(uv, 1, 1);
    output.rt3 = float4(1, 1, 1, 1);

    return output;
}
#endif