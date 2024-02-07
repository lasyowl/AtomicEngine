RWTexture2D<float4> outTexture : register(u0);
RaytracingAccelerationStructure sceneAS : register(t0);

[shader("raygeneration")]
void RayGeneration()
{
    uint2 launchIdx = DispatchRaysIndex().xy;
    uint2 dimensions = DispatchRaysDimensions().xy;

    outTexture[launchIdx] = float4(0.4f, 0.2f, 0.0f, 1.0f);
}

struct Payload
{
    float4 color;
};

struct BuiltInAttribute
{
    float x;
    float y;
};

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