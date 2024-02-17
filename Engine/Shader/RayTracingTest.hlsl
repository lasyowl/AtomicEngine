#define PI 3.14

cbuffer PerFrameConstants : register (b0)
{
    float4x4 viewProjection;
    float4x4 viewProjectionInv;
    float3 viewPosition;
}

struct RayTraceInstanceContext
{
	float4x4 matRotation;
	uint normalOffset;
	uint indexOffset;
    uint2 padding;
};

struct PBRMaterial
{
    float3 baseColor;
    float3 normal;
    float roughness;
    float metalness;
};

RWTexture2D<float4> renderTarget : register(u0);
RaytracingAccelerationStructure topLevelAS : register(t0);
StructuredBuffer<float3> normals : register(t1);
StructuredBuffer<uint> indices : register(t2);
StructuredBuffer<RayTraceInstanceContext> instanceContexts : register(t3);
StructuredBuffer<PBRMaterial> materials : register(t4);

struct Payload
{
    float4 color;
    uint recursion;
};

struct ShadowPayload
{
    bool bHit;
};

float RandomValue(inout uint state)
{
    state = state * 747796405 + 2891336453;
    uint result = ((state >> ((state >> 28) + 4)) ^ state) * 277803737;
    result = (result >> 22) ^ result;
    return result / 4294967295.0;
}

// Generate a ray in world space for a camera pixel corresponding to an index from the dispatched 2D grid.
inline void GenerateCameraRay(uint2 index, out float3 origin, out float3 target)
{
    float2 xy = index + 0.5f; // center in the middle of the pixel.
    float2 screenPos = xy / DispatchRaysDimensions().xy * 2.0 - 1.0;

    // Invert Y for DirectX-style coordinates.
    screenPos.y = -screenPos.y;

    // Unproject the pixel coordinate into a ray.
    float4 world = mul(viewProjectionInv, float4(screenPos, 0, 1));

    world.xyz /= world.w;
    origin = viewPosition;
    target = world.xyz;
}

[shader("raygeneration")]
void RayGeneration()
{
    uint2 rayIdx = DispatchRaysIndex().xy;

    float3 target;
    float3 origin;
    
    // Generate a ray for a camera pixel corresponding to an index from the dispatched 2D grid.
    GenerateCameraRay(rayIdx, origin, target);
    
    //float3 rayDir = normalize(target - origin);

    // RayDesc ray;
    // ray.Origin = origin;
    // ray.Direction = rayDir;
    // ray.TMin = 0.00001f;
    // ray.TMax = 10000.0f;

    // Payload payload;
    // payload.recursion = 3;
    // TraceRay(
    //     topLevelAS,
    //     RAY_FLAG_NONE,
    //     0xFF,
    //     0,
    //     0,
    //     0,
    //     ray,
    //     payload
    // );

    //renderTarget[rayIdx] = payload.color;

    { /* Depth of field */
        float4 color = 0;

        uint seed = rayIdx.x + rayIdx.y * 1920;
        for(uint iter = 0; iter < 5; ++iter)
        {
            //float3 randomDir = normalize(float3(RandomValue(seed), RandomValue(seed), RandomValue(seed)));
            float2 jitterXY = normalize(float2(RandomValue(seed), RandomValue(seed)));
            float3 jitter = 0.0001f * float3(jitterXY, 0);
            float3 direction = normalize(target - origin + jitter);

            RayDesc ray;
            ray.Origin = origin;
            ray.Direction = direction;//normalize(300 * rayDir + randomDir);
            ray.TMin = 0.00001f;
            ray.TMax = 10000.0f;

            Payload payload;
            payload.recursion = 3;
            TraceRay(
                topLevelAS,
                RAY_FLAG_NONE,
                0xFF,
                0,
                0,
                0,
                ray,
                payload
            );

            color += payload.color;
        }

        renderTarget[rayIdx] = color / color.w;
    }
}

struct BuiltInAttribute
{
    float2 barycentrics;
};

float TrowbridgeReitzGGX(float3 N, float3 H, float roughness)
{
    float numerator = roughness * roughness;
    float NdotH = max(dot(N, H), 0.0f);
    float denominator = PI * pow(pow(NdotH, 2.0f) * (roughness * roughness - 1.0f) + 1.0f, 2.0f);
    denominator = max(denominator, 0.000001f);

    return numerator / denominator;
}

float FresnelSchlickReflectance(float3 V, float3 H, float F0)
{
    return F0 + (1.0f - F0) * pow(1 - max(dot(V, H), 0.0f), 5.0f);
}

float SmithSchlickGGX(float3 N, float3 V, float3 L, float roughness)
{
    roughness += 1;
    float k = (roughness * roughness) / 8.0f;

    float NdotV = max(dot(N, V), 0.0f);
    float NdotL = max(dot(N, L), 0.0f);

    float ggx0 = NdotV / (NdotV * (1.0f - k) + k);
    float ggx1 = NdotL / (NdotL * (1.0f - k) + k);

    return ggx0 * ggx1;
}

float4 PBR(float3 baseColor, float3 lightColor, float3 N, float3 V, float3 L, float distribution, float fresnel, float geometry, float metalness)
{
    // float3 N = normalize(normal);
    // float3 V = normalize(viewDirection);
    // float3 L = normalize(lightDirection);
    // float3 H = normalize(V + L);
    // float R = roughness;
    // float A = 1.0f - roughness;
    // float F0 = reflectance;

    // cale emissive
    float3 emissive = baseColor * 0.2f;

    // calc diffuse
    float3 lambert = baseColor / PI;

    // calc Ks
    float ks = fresnel;
    // calc Kd
    float kd = (1.0f - ks) * (1.0f - metalness);

    float3 cookTorranceNumerator = distribution * geometry * fresnel;
    float3 cookTorranceDenominator = max(4.0f * max(dot(V, N), 0.0f) * max(dot(L, N), 0.0f), 0.000001f);
    float3 cookTorrance = cookTorranceNumerator / cookTorranceDenominator;

    float3 brdf = kd * lambert + cookTorrance;
    float3 color = emissive + brdf * lightColor * max(dot(L, N), 0.0f);

    return float4(color, 1);
}

[shader("closesthit")]
void Hit(inout Payload inPayload : SV_Payload, BuiltInAttribute attr)
{
    RayTraceInstanceContext instanceContext = instanceContexts[InstanceID()];
    uint index = instanceContext.indexOffset + PrimitiveIndex() * 3;
    
    PBRMaterial material = materials[InstanceID()];

    float3 normal0 = normals[instanceContext.normalOffset + indices[index + 0]];
    float3 normal1 = normals[instanceContext.normalOffset + indices[index + 1]];
    float3 normal2 = normals[instanceContext.normalOffset + indices[index + 2]];
    float3 normal = normalize(normal0 + attr.barycentrics.x * (normal1 - normal0) + attr.barycentrics.y * (normal2 - normal0));
    normal = mul(float4(normal, 1.0f), instanceContext.matRotation).xyz;

    float rayT = RayTCurrent();
    float3 rayDirection = WorldRayDirection();
    float3 hitPosition = WorldRayOrigin() + rayT * rayDirection;

    float3 lightPosition = float3(0, 10, 5);
    float3 lightDirection = normalize(hitPosition - lightPosition);
    float lightIntensity = max(1.0f / length(hitPosition - lightPosition), 5.0f);
    //float3 directionalLight = normalize(float3(1, -1, -1));
    float3 lightColor = lightIntensity * float3(1, 1, 1);
    
    float3 H = normalize(-rayDirection - lightDirection);
    float glossiness = 1.0f - material.roughness;
    // calc specular(Cook-Torrance, DFG)
    // D : Trowbridge-Reitz GGX
    // F : Fresnel-Schlick approximation
    // G : Smith's Schlick GGX
    float distribution = TrowbridgeReitzGGX(normal, H, material.roughness);
    float fresnel = FresnelSchlickReflectance(-rayDirection, H, glossiness);
    float geometry = SmithSchlickGGX(normal, -rayDirection, -lightDirection, material.roughness);

    inPayload.color = PBR(material.baseColor, lightColor, normal, -rayDirection, -lightDirection, distribution, fresnel, geometry, material.metalness);
    
    if(inPayload.recursion > 0 && HitKind() == 254)
    {
        // RayDesc shadowRay;
        // shadowRay.Origin = hitPosition;
        // shadowRay.Direction = -lightDirection;
        // shadowRay.TMin = 0.0001f;
        // shadowRay.TMax = length(hitPosition - lightPosition);

        // ShadowPayload shadowPayload;
        // shadowPayload.bHit = true;

        // TraceRay(
        //     topLevelAS,
        //     RAY_FLAG_NONE,
        //     0xFF,
        //     0,
        //     0,
        //     1,
        //     shadowRay,
        //     shadowPayload
        // );

        RayDesc reflectionRay;
        reflectionRay.Origin = hitPosition;
        reflectionRay.Direction = reflect(rayDirection, normal);
        reflectionRay.TMin = 0.0001f;
        reflectionRay.TMax = 10000.0f;

        Payload payload;
        payload.recursion = inPayload.recursion - 1;

        TraceRay(
            topLevelAS,
            RAY_FLAG_NONE,
            0xFF,
            0,
            0,
            0,
            reflectionRay,
            payload
        );

        inPayload.color.xyz += fresnel * payload.color.xyz;

        //float factor = shadowPayload.bHit ? 0.3f : 1.0f;
        //inPayload.color.xyz *= factor;
    }

    inPayload.color = lerp(inPayload.color, float4(0.3f, 0.3f, 0.7f, 1), 1.0f - exp(-0.000002 * rayT * rayT * rayT));
}

[shader("miss")]
void Miss(inout Payload payload : SV_Payload)
{
    payload.color = float4(0.3f, 0.3f, 0.7f, 1);
}

[shader("miss")]
void ShadowMiss(inout ShadowPayload payload : SV_Payload)
{
    payload.bHit = false;
}