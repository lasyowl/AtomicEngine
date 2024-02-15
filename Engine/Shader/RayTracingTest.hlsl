cbuffer PerFrameConstants : register (b0)
{
    float4x4 viewProjection;
    float4x4 viewProjectionInv;
    float3 viewPosition;
}

RWTexture2D<float4> renderTarget : register(u0);
RaytracingAccelerationStructure topLevelAS : register(t0);
StructuredBuffer<float3> normals : register(t1);
StructuredBuffer<uint> indices : register(t2);
StructuredBuffer<uint2> dataOffsets : register(t3);

struct Payload
{
    float4 color;
    uint recursion;
};

// Generate a ray in world space for a camera pixel corresponding to an index from the dispatched 2D grid.
inline void GenerateCameraRay(uint2 index, out float3 origin, out float3 direction)
{
    float2 xy = index + 0.5f; // center in the middle of the pixel.
    float2 screenPos = xy / DispatchRaysDimensions().xy * 2.0 - 1.0;

    // Invert Y for DirectX-style coordinates.
    screenPos.y = -screenPos.y;

    // Unproject the pixel coordinate into a ray.
    float4 world = mul(viewProjectionInv, float4(screenPos, 0, 1));

    world.xyz /= world.w;
    origin = viewPosition;
    direction = normalize(world.xyz - origin);
}

[shader("raygeneration")]
void RayGeneration()
{
    uint2 rayIdx = DispatchRaysIndex().xy;

    float3 rayDir;
    float3 origin;
    
    // Generate a ray for a camera pixel corresponding to an index from the dispatched 2D grid.
    GenerateCameraRay(rayIdx, origin, rayDir);

    RayDesc ray;
    ray.Origin = origin;
    ray.Direction = rayDir;
    ray.TMin = 0.01f;
    ray.TMax = 1000.0f;

    Payload payload;
    payload.recursion = 1;
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

    renderTarget[rayIdx] = payload.color;
}

struct BuiltInAttribute
{
    float2 barycentrics;
};

[shader("closesthit")]
void Hit(inout Payload inPayload : SV_Payload, BuiltInAttribute attr)
{
    uint2 dataOffset = dataOffsets[InstanceID()];
    uint index = dataOffset.y + PrimitiveIndex() * 3;
    
    float3 normal0 = normals[dataOffset.x + indices[index + 0]];
    float3 normal1 = normals[dataOffset.x + indices[index + 1]];
    float3 normal2 = normals[dataOffset.x + indices[index + 2]];
    float3 normal = normal0 + attr.barycentrics.x * (normal1 - normal0) + attr.barycentrics.y * (normal2 - normal0);

    float3 objColor = InstanceID() == 0 ? float3(1, 0, 0) : InstanceID() == 1 ? float3(0, 1, 0) : float3(0, 0, 1);

    float3 directionalLight = float3(0, -1, 1);
    float3 diffuse = objColor * max(0, dot(directionalLight, -normal));
    float3 specular = objColor * max(0, pow(dot(WorldRayDirection(), -normal), 20));

    inPayload.color = float4(diffuse + specular, 1.0f);

    if(inPayload.recursion > 0)
    {
        RayDesc ray;
        ray.Origin = WorldRayOrigin() + RayTCurrent() * WorldRayDirection();
        ray.Direction = reflect(WorldRayDirection(), normal);
        ray.TMin = 0.01f;
        ray.TMax = 1000.0f;

        Payload payload;
        payload.recursion = inPayload.recursion - 1;

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

        inPayload.color += payload.color;
    }
}

[shader("miss")]
void Miss(inout Payload payload : SV_Payload)
{
    payload.color = float4(0.2f, 0.2f, 0.6f, 1);
}