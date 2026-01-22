RWBuffer<uint> data : register(u0);

[numthreads(64, 1, 1)]
void CS_main(uint3 DispatchThreadID : SV_DispatchThreadID)
{
    data[DispatchThreadID.x] = DispatchThreadID.x;
}
