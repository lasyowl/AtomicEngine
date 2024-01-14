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

struct VertexShaderOutput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

////////////////////////////////
// DirectionalLight
////////////////////////////////
VertexShaderOutput VS_DirectionalLight(
    float3 position : POSITION,
    float2 uv : TEXCOORD)
{
    VertexShaderOutput output;

    output.position = float4(position, 1.0f);
    output.uv = uv;

    return output;
}

float4 PS_DirectionalLight (
    float4 position : SV_POSITION,
    float2 uv : TEXCOORD) : SV_TARGET
{
    float depth = gDepth.Sample(smp, uv).r;
    float2 screenPos = float2(position.x / 1920.0f, 1.0f - position.y / 1080.0f);
    float4 clipPos = float4(screenPos * 2.0f - 1.0f, depth, 1.0f);
    float4 worldPosPerspective = mul(viewProjectionInv, clipPos);
    float3 worldPos = worldPosPerspective.xyz / worldPosPerspective.w;

    if(gDiffuse.Sample(smp, uv).r == 0) discard;

    float3 lightColor = float3(0.4, 0.4, 0.7);
    float3 lightNormal = normalize(float3(0, 0, 1));

    float3 surfaceNormal = normalize(2.0f * gNormal.Sample(smp, uv).xyz - 1.0f);
    float3 diffuse = lightColor * dot(-lightNormal, surfaceNormal);

    float3 reflectNormal = reflect(lightNormal, surfaceNormal);
    float3 screenNormal = normalize(viewPosition - worldPos);
    float3 specular = lightColor * pow(max(0, dot(reflectNormal, screenNormal)), 30);

    return float4(diffuse + specular, 1);
}

////////////////////////////////
// PointLight
////////////////////////////////
cbuffer LightConstants : register (b1)
{
    float4x4 model;
    float3 lightOrigin;
    float lightIntensity;
}

VertexShaderOutput VS_PointLight(
    float3 position : POSITION,
    float2 uv : TEXCOORD)
{
    VertexShaderOutput output;

    output.position = mul(mul(float4(position, 1.0f), model), viewProjection);
    output.uv = uv;

    return output;
}

float4 PS_PointLight (
    float4 position : SV_POSITION,
    float2 uv : TEXCOORD) : SV_TARGET
{
    float2 screenPos = float2(position.x / 1920.0f, position.y / 1080.0f);
    float2 screenPosYInv = float2(screenPos.x, 1.0f - screenPos.y);
    float depth = gDepth.Sample(smp, screenPos).r;
    float4 clipPos = float4(screenPosYInv * 2.0f - 1.0f, depth, 1.0f);
    float4 worldPosPerspective = mul(viewProjectionInv, clipPos);
    float3 worldPos = worldPosPerspective.xyz / worldPosPerspective.w;

    if(gDiffuse.Sample(smp, screenPos).r == 0) discard;

    float3 lightNormal = normalize(worldPos - lightOrigin);
    float lightDistance = distance(worldPos, lightOrigin);
    float attenuation = 1.0f - min(1.0f, lightDistance / lightIntensity);
    float3 lightColor = attenuation * float3(1, 0, 0);

    float3 surfaceNormal = normalize(2.0f * gNormal.Sample(smp, screenPos).xyz - 1.0f);
    float3 diffuse = lightColor * dot(-lightNormal, surfaceNormal);

    float3 reflectNormal = reflect(lightNormal, surfaceNormal);
    float3 screenNormal = normalize(viewPosition - worldPos);
    float3 specular = lightColor * pow(max(0, dot(reflectNormal, screenNormal)), 30);   

    //return float4(lightDistance, lightDistance, lightDistance/* * diffuse*/, 1);
    return float4(diffuse + specular, 1);
}

////////////////////////////////
// Light Combine
////////////////////////////////
VertexShaderOutput VS_LightCombine(
    float3 position : POSITION,
    float2 uv : TEXCOORD)
{
    VertexShaderOutput output;

    output.position = float4(position, 1.0f);
    output.uv = uv;

    return output;
}

float4 PS_LightCombine (
    float4 position : SV_POSITION,
    float2 uv : TEXCOORD) : SV_TARGET
{
    float2 screenPos = float2(position.x / 1920.0f, position.y / 1080.0f);
    float2 screenPosYInv = float2(screenPos.x, 1.0f - screenPos.y);
    float depth = gDepth.Sample(smp, screenPos).r;
    float4 clipPos = float4(screenPosYInv * 2.0f - 1.0f, depth, 1.0f);
    float4 worldPosPerspective = mul(viewProjectionInv, clipPos);
    float3 worldPos = worldPosPerspective.xyz / worldPosPerspective.w;

    if(gDiffuse.Sample(smp, screenPos).r == 0) discard;

    float3 lightNormal = normalize(worldPos - lightOrigin);
    float lightDistance = distance(worldPos, lightOrigin);
    float attenuation = 1.0f - min(1.0f, lightDistance / lightIntensity);
    float3 lightColor = attenuation * float3(1, 0, 0);

    float3 surfaceNormal = normalize(2.0f * gNormal.Sample(smp, screenPos).xyz - 1.0f);
    float3 diffuse = lightColor * dot(-lightNormal, surfaceNormal);

    float3 reflectNormal = reflect(lightNormal, surfaceNormal);
    float3 screenNormal = normalize(viewPosition - worldPos);
    float3 specular = lightColor * pow(max(0, dot(reflectNormal, screenNormal)), 30);   

    //return float4(lightDistance, lightDistance, lightDistance/* * diffuse*/, 1);
    return float4(diffuse + specular, 1);
}