cbuffer PerFrameConstants : register (b0)
{
    float4x4 viewProjection;
    float4x4 viewProjectionInv;
    float3 viewPosition;
}

Texture2D gDiffuse : register (t0);
Texture2D gNormal : register (t1);
Texture2D gBuffer2 : register (t2);
Texture2D gDepth : register (t3);
SamplerState smp { AddressU = Wrap; AddressV = Wrap; };

StructuredBuffer<float3> lights : register (t4);

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
    float depth = gDepth.Sample(smp, uv).r;
    float2 screenPos = float2(position.x / 1920.0f, 1.0f - position.y / 1080.0f);
    float4 clipPos = float4(screenPos * 2.0f - 1.0f, depth, 1.0f);
    float4 worldPosPerspective = mul(viewProjectionInv, clipPos);
    float3 worldPos = worldPosPerspective.xyz / worldPosPerspective.w;

    if(gDiffuse.Sample(smp, uv).r == 0) discard;

    //return float4(lights[0].xyz, 1);

    /* Directional light */
    float3 dirLightColor = float3(0.4, 0.4, 0.7);
    float3 dirLightNormal = normalize(float3(0, 0, 1));

    float3 surfaceNormal = normalize(2.0f * gNormal.Sample(smp, uv).xyz - 1.0f);
    float3 diffuse = dirLightColor * dot(-dirLightNormal, surfaceNormal);

    float3 reflectNormal = reflect(dirLightNormal, surfaceNormal);
    float3 screenNormal = normalize(viewPosition - worldPos);//float3(0, 0, -1);
    float3 specular = dirLightColor * pow(max(0, dot(reflectNormal, screenNormal)), 30);

    float3 lightPos = float3(0, 0, -2);
    float lightDistance = 1 - distance(lightPos, worldPos) / 3.0f;
    //float lightDistance = 1 - distance(lightPos, clipPos.xyz);

    //return float4(lightDistance, lightDistance, lightDistance/* * diffuse*/, 1);
    return float4(diffuse + specular, 1);
}