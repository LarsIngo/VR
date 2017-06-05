
// Meta data.
struct MetaData
{
    float3 position;
    float3 velocity;
    float2 scale;
    float lifetime;
    float3 color;
    uint emittIndex;
    int emittPointIndex;
};
// Meta buffer.
StructuredBuffer<MetaData> g_MetaBuffer : register(t0);

// Input.
StructuredBuffer<float3> g_EmittPointsBuffer : register(t1);

// Output.
RWStructuredBuffer<float3> g_OutPositionBuffer : register(u0);
RWStructuredBuffer<float2> g_OutScaleBuffer : register(u1);
RWStructuredBuffer<float3> g_OutVelocityBuffer : register(u2);
RWStructuredBuffer<float> g_OutLifetimeBuffer : register(u3);
RWStructuredBuffer<float3> g_OutColorBuffer : register(u4);

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
    float3 position = metaData.position;
    float3 velocity = metaData.velocity;
    float2 scale = metaData.scale;
    float lifetime = metaData.lifetime;
    float3 color = metaData.color;
    uint emittIndex = metaData.emittIndex;
    int emittPointIndex = metaData.emittPointIndex;

    float3 offset = float3(0.f, 0.f, 0.f);
    if (emittPointIndex >= 0)
    {
        offset = g_EmittPointsBuffer[emittPointIndex];
    }

    g_OutPositionBuffer[emittIndex] = position + offset;
    g_OutScaleBuffer[emittIndex] = scale;
    g_OutVelocityBuffer[emittIndex] = velocity;
    g_OutLifetimeBuffer[emittIndex] = lifetime;
    g_OutColorBuffer[emittIndex] = color;
}
