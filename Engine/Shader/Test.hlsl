#if D3D12_SAMPLE_BASIC
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

    output.position.xyz = position * 0.5f;
    output.position.w = 1;
    output.uv = uv;

    return output;
}

float4 PS_main (float4 position : SV_POSITION,
                float3 normal : NORMAL,
                float2 uv : TEXCOORD) : SV_TARGET
{
    return float4(normal, 1);
}
#elif D3D12_SAMPLE_CONSTANT_BUFFER
cbuffer PerFrameConstants : register (b0)
{
    float4x4 viewProjection;
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

    output.position = mul(float4(position, 1.0f), viewProjection);
    output.normal = normal;
    output.uv = uv;

    return output;
}

float4 PS_main (float4 position : SV_POSITION,
                float3 normal : NORMAL,
                float2 uv : TEXCOORD) : SV_TARGET
{
    float4 color = testTex.Sample(smp, uv);
    return float4(color.rgb, 1);
    return float4(normal, 1);
}
#elif D3D12_SAMPLE_TEXTURE
cbuffer PerFrameConstants : register (b0)
{
    float4 scale;
}

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

    output.position.xyz = position * 0.5f;
    output.position.w = 1;
    output.uv = uv;

    return output;
}

Texture2D<float4> texture : register(t0);
SamplerState texureSampler      : register(s0);

float4 PS_main (float4 position : SV_POSITION,
                float2 uv : TEXCOORD) : SV_TARGET
{
    return texture.Sample (texureSampler, uv);
}
#endif