#if D3D12_SAMPLE_BASIC
struct VertexShaderOutput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

VertexShaderOutput VS_main(
    float4 position : POSITION,
    float2 uv : TEXCOORD)
{
    VertexShaderOutput output;

    output.position = position * 0.5f;
    output.position.w = 1;
    output.uv = uv;

    return output;
}

float4 PS_main (float4 position : SV_POSITION,
                float2 uv : TEXCOORD) : SV_TARGET
{
    return float4(uv, 0, 1);
}
#elif D3D12_SAMPLE_CONSTANT_BUFFER
cbuffer PerFrameConstants : register (b0)
{
    float4x4 viewProjection;
}

struct VertexShaderOutput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

VertexShaderOutput VS_main(
    float4 position : POSITION,
    float2 uv : TEXCOORD)
{
    VertexShaderOutput output;

    output.position = position * 0.5f;
    output.position.w = 1;
    output.position = mul(output.position, viewProjection);
    
    output.uv = uv;

    return output;
}

float4 PS_main (float4 position : SV_POSITION,
                float2 uv : TEXCOORD) : SV_TARGET
{
    return float4(uv, 0, 1);
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
    float4 position : POSITION,
    float2 uv : TEXCOORD)
{
    VertexShaderOutput output;

    output.position = position;
    output.position.xy *= scale.x;
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