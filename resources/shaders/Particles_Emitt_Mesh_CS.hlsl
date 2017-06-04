
// Meta data.
struct MetaData
{
    float dt;
    uint particleCount;
    float pad[2];
};
// Meta buffer.
StructuredBuffer<MetaData> g_MetaBuffer : register(t0);

// Input.
StructuredBuffer<float3> g_EmittPointsBuffer : register(t1);

// Output.
RWStructuredBuffer<float3> g_OutPositionBuffer : register(u0);
RWStructuredBuffer<float3> g_OutOldPositionBuffer : register(u1);
RWStructuredBuffer<float3> g_OutVelocityBuffer : register(u2);
RWStructuredBuffer<float> g_OutLifetimeBuffer : register(u3);

uint WangHash(uint seed)
{
    seed = (seed ^ 61) ^ (seed >> 16);
    seed *= 9;
    seed = seed ^ (seed >> 4);
    seed *= 0x27d4eb2d;
    seed = seed ^ (seed >> 15);
    return seed;
}

[numthreads(128, 1, 1)]
void main(uint3 threadID : SV_DispatchThreadID)
{
    MetaData metaData = g_MetaBuffer[0];
    uint particleCount = metaData.particleCount;
    float dt = metaData.dt;
    uint tID = threadID.x;
    uint rand = WangHash(tID);

    float3 self_position = g_InPositionBuffer[tID];
    float3 new_old_position = self_position;
    float3 self_oldPosition = g_InOldPositionBuffer[tID];
    float3 self_velocity = g_InVelocityBuffer[tID];
    float self_lifetime = g_InLifetimeBuffer[tID];

    // +++ PHYSICS +++ //
    self_position += float3(0.f, 1.f, 0.f) * dt;
    //self.velocity.xyz -= self.velocity.xyz * 0.5f * dt;
    //self.velocity.xyz += self.acceleration.xyz * dt;
    // --- PHYSICS --- //

    /*
    self.position.xyz = self.position.xyz + self.velocity.xyz * dt;
    float velocityFactor = length(self.velocity.xyz);
    //float accelerationFactor = length(self.acceleration.xyz);
    self.color = float4(lenFactor, 0.f, 0.2f, clamp(velocityFactor / 25.f, 0.2f, 1.f));
    float scaleFactor = clamp(velocityFactor / 25.f, 0.5f, 1.5f);
    self.scale = float4(scaleFactor, scaleFactor, 0.f, 0.f);
    */

    g_OutPositionBuffer[tID] = self_position;
    g_OutOldPositionBuffer[tID] = new_old_position;
    g_OutVelocityBuffer[tID] = self_velocity;
    g_OutLifetimeBuffer[tID] = self_lifetime;
}
