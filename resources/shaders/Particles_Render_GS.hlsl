// Input.
struct GSInput
{
    float3 position : POSITION;
    float2 scale : SCALE;
    float3 velocity : VELOCITY;
    float lifetime : LIFETIME;
    float3 color : COLOR;
};

// Output.
struct GSOutput
{
    float4 position : SV_POSITION;
    float2 scale : SCALE;
    float3 velocity : VELOCITY;
    float lifetime : LIFETIME;
    float3 color : COLOR;
    float3 worldPosition : WORLDPOSITION;
    float2 uv : UV;
};

// Meta data.
struct MetaData
{
    float4x4 vpMatrix;
    float3 lensPosition;
    float3 lensUpDirection;
    float pad[2];
};
// Meta buffer.
StructuredBuffer<MetaData> g_MetaBuffer : register(t0);

[maxvertexcount(4)]
void main(point GSInput input[1], inout TriangleStream<GSOutput> TriStream)
{
    float lifetime = input[0].lifetime;
    if (lifetime > 0.f)
    {
        GSOutput output;

        MetaData metaData = g_MetaBuffer[0];
        float4x4 vpMatrix = metaData.vpMatrix;
        float3 lensPosition = metaData.lensPosition;
        float3 lensUpDirection = metaData.lensUpDirection;

        float3 worldPosition = input[0].position.xyz;
        float2 scale = input[0].scale;

        float3 particleFrontDirection = normalize(lensPosition - worldPosition);
        float3 paticleSideDirection = cross(particleFrontDirection, lensUpDirection);
        float3 paticleUpDirection = cross(paticleSideDirection, particleFrontDirection);

        for (uint i = 0; i < 4; ++i)
        {
            float x = i == 1 || i == 3;
            float y = i == 0 || i == 1;
            output.position.xyz = worldPosition + paticleSideDirection * (x * 2.f - 1.f) * scale.x + paticleUpDirection * (y * 2.f - 1.f) * scale.y;
            output.position.w = 1.f;
            output.scale = scale;
            output.velocity = input[0].velocity;
            output.lifetime = lifetime;
            output.color = input[0].color;
            output.worldPosition = output.position.xyz;
            output.position = mul(output.position, vpMatrix);
            output.uv = float2(x, 1.f - y);

            TriStream.Append(output);
        }
    }
}
