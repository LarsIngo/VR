// Particle.
struct Particle
{
    float3 position : POSITION;
    float3 oldPosition : OLDPOSITION;
    float3 velocity : VELOCITY;
    float lifetime : LIFETIME;
};

StructuredBuffer<float3> g_PositionBuffer : register(t0);
StructuredBuffer<float3> g_OldPositionBuffer : register(t1);
StructuredBuffer<float3> g_VelocityBuffer : register(t2);
StructuredBuffer<float> g_LifetimeBuffer : register(t3);

Particle main(uint vID : SV_VertexID)
{
    Particle self;
    
    self.position = g_PositionBuffer[vID];
    self.oldPosition = g_OldPositionBuffer[vID];
    self.velocity = g_VelocityBuffer[vID];
    self.lifetime = g_LifetimeBuffer[vID];

    return self;
}
