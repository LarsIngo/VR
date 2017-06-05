
// Meta data.
struct MetaData
{
    uint randomNumber;
    uint emittIndex;
};
// Meta buffer.
StructuredBuffer<MetaData> g_MetaBuffer : register(t0);

// Input.
//StructuredBuffer<float3> g_EmittPointsBuffer : register(t1);

// Output.
RWStructuredBuffer<float3> g_OutPositionBuffer : register(u0);
RWStructuredBuffer<float3> g_OutOldPositionBuffer : register(u1);
RWStructuredBuffer<float3> g_OutVelocityBuffer : register(u2);
RWStructuredBuffer<float> g_OutLifetimeBuffer : register(u3);

//uint WangHash(uint seed)
//{
//    seed = (seed ^ 61) ^ (seed >> 16);
//    seed *= 9;
//    seed = seed ^ (seed >> 4);
//    seed *= 0x27d4eb2d;
//    seed = seed ^ (seed >> 15);
//    return seed;
//}

[numthreads(1, 1, 1)]
void main(uint3 threadID : SV_DispatchThreadID)
{
    MetaData metaData = g_MetaBuffer[0];
    uint randomNumber = metaData.randomNumber;
    uint emittIndex = metaData.emittIndex;

    g_OutPositionBuffer[emittIndex] = float3(3,3,3);
    g_OutOldPositionBuffer[emittIndex] = float3(3, 3, 3);
    g_OutVelocityBuffer[emittIndex] = float3(0, 1, 0);
    g_OutLifetimeBuffer[emittIndex] = float(10);
}
