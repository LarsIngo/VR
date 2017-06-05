
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
StructuredBuffer<float3> g_InPositionBuffer : register(t1);
StructuredBuffer<float2> g_InScaleBuffer : register(t2);
StructuredBuffer<float3> g_InVelocityBuffer : register(t3);
StructuredBuffer<float> g_InLifetimeBuffer : register(t4);
StructuredBuffer<float3> g_InColorBuffer : register(t5);

// Output.
RWStructuredBuffer<float3> g_OutPositionBuffer : register(u0);
RWStructuredBuffer<float2> g_OutScaleBuffer : register(u1);
RWStructuredBuffer<float3> g_OutVelocityBuffer : register(u2);
RWStructuredBuffer<float> g_OutLifetimeBuffer : register(u3);
RWStructuredBuffer<float3> g_OutColorBuffer : register(u4);

[numthreads(128, 1, 1)]
void main(uint3 threadID : SV_DispatchThreadID)
{
    MetaData metaData = g_MetaBuffer[0];
    uint particleCount = metaData.particleCount;
    float dt = metaData.dt;
    uint tID = threadID.x;

    float3 self_position = g_InPositionBuffer[tID];
    float2 self_scale = g_InScaleBuffer[tID];
    float3 self_velocity = g_InVelocityBuffer[tID];
    float self_lifetime = g_InLifetimeBuffer[tID];
    float3 self_color = g_InColorBuffer[tID];

    // +++ PHYSICS +++ //
    self_position += self_velocity * dt;
    self_lifetime -= dt;

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
    g_OutScaleBuffer[tID] = self_scale;
    g_OutVelocityBuffer[tID] = self_velocity;
    g_OutLifetimeBuffer[tID] = self_lifetime;
    g_OutColorBuffer[tID] = self_color;
}
