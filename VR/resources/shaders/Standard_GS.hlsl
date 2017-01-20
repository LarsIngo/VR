struct Input
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 uv : UV;
};
 
struct Output
{
    float4 position : SV_POSITION;
    float3 worldPosition : WORLDPOSITION;
    float3 normal : NORMAL;
    float2 uv : UV;
};

struct Meta
{
    float4x4 modelMatrix;
    float4x4 mvpMatrix;
};
StructuredBuffer<Meta> g_Meta : register(t0);

[maxvertexcount(3)]
void main(triangle Input input[3], inout TriangleStream<Output> outStream)
{
    Output output;

    Meta meta = g_Meta[0];
 
    for (uint i = 0; i < 3; ++i)
    {
        output.position = mul(float4(input[i].position, 1.f), meta.mvpMatrix);
        output.worldPosition = mul(float4(input[i].position, 1.f), meta.modelMatrix).xyz;
        output.normal = input[i].normal;
        output.uv = input[i].uv;
        outStream.Append(output);
    }
}
 