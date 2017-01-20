struct Input
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 uv : UV;
};
 
struct Output
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 uv : UV;
};

Output main(Input input)
{
    Output output;
 
    output.position = input.position;
    output.normal = input.normal;
    output.uv = input.uv;

    return output;
}
 