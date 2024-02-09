RWTexture2D<float4> renderTarget : register(u0);
RaytracingAccelerationStructure topLevelAS : register(t0);

struct Payload
{
    float4 color;
};

struct BuiltInAttribute
{
    float BaryX;
    float BaryY;
};

[shader("raygeneration")]
void RayGeneration()
{
    uint2 launchIdx = DispatchRaysIndex().xy;
    uint2 dimensions = DispatchRaysDimensions().xy;

    float2 ndc = ((float2)launchIdx + 0.5f) / (float2)dimensions * 2.0f - 1.0f;

    RayDesc ray;
    ray.Origin = float3(ndc.x, -ndc.y, -1.0f);
    ray.Direction = float3(0.0f, 0.0f, 1.0f);
    ray.TMin = 0.1f;
    ray.TMax = 1000.0f;

    Payload payload;
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

    renderTarget[launchIdx] = /*renderTarget[launchIdx] **/ payload.color;
}

[shader("closesthit")]
void Hit(inout Payload payload : SV_Payload, BuiltInAttribute attr)
{
    payload.color = 1.0f;
}

[shader("miss")]
void Miss(inout Payload payload : SV_Payload)
{
    payload.color = 0.0f;
}