RWTexture2D<float4> outTexture : register(u0);

[shader("raygeneration")]
void RayGeneration()
{
    uint2 launchIdx = DispatchRaysIndex().xy;
    uint2 dimensions = DispatchRaysDimensions().xy;

    outTexture[launchIdx] = float4(0.4f, 0.2f, 0.0f, 1.0f);
}