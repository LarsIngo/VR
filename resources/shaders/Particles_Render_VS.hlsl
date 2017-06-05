// Particle.
struct Particle
{
    float3 position : POSITION;
    float2 scale : SCALE;
    float3 velocity : VELOCITY;
    float lifetime : LIFETIME;
    float3 color : COLOR;
};

StructuredBuffer<float3> g_PositionBuffer : register(t0);
StructuredBuffer<float3> g_ScaleBuffer : register(t1);
StructuredBuffer<float3> g_VelocityBuffer : register(t2);
StructuredBuffer<float> g_LifetimeBuffer : register(t3);
StructuredBuffer<float3> g_ColorBuffer : register(t4);

Particle main(uint vID : SV_VertexID)
{
    Particle self;
    
    self.position = g_PositionBuffer[vID];
    self.scale = g_ScaleBuffer[vID];
    self.velocity = g_VelocityBuffer[vID];
    self.lifetime = g_LifetimeBuffer[vID];
    self.color = g_ColorBuffer[vID];

    return self;
}
