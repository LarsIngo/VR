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

Output main(Input input)
{
    Output output;
 
    output.position = float4(input.position, 1.f);
    output.worldPosition = input.position;
    output.normal = input.normal;
    output.uv = input.uv;

    return output;
}
 