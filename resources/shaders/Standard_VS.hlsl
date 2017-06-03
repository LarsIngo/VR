
StructuredBuffer<uint> g_IndexBuffer : register(t0);

StructuredBuffer<float3> g_PositionBuffer : register(t1);
StructuredBuffer<float2> g_UVBuffer : register(t2);
StructuredBuffer<float3> g_NormalBuffer : register(t3);
StructuredBuffer<float3> g_TangentBuffer : register(t4);
 
struct Output
{
    float3 position : POSITION;
    float2 uv : UV;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
};

Output main(uint vID : SV_VertexID)
{
    Output output;

    uint index = g_IndexBuffer[vID];
 
    output.position = g_PositionBuffer[index];
    output.uv = g_UVBuffer[index];
    output.normal = g_NormalBuffer[index];
    output.tangent = g_TangentBuffer[index];

    return output;
}
 