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

[maxvertexcount(3)]
void main(triangle Input input[3], inout TriangleStream<Output> outStream)
{
    Output output;
 
    for (uint i = 0; i < 3; ++i)
    {
        output.position = float4(input[i].position, 1.f); //mul(float4(position, 1.f), wvpMatrix);
        output.worldPosition = input[i].position; //mul(float4(position, 1.f), worldMatrix).xyz;
        output.normal = input[i].normal;
        output.uv = input[i].uv;
        outStream.Append(output);
    }
}
 