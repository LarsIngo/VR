struct Input
{
    float4 position : SV_POSITION;
    float3 worldPosition : WORLDPOSITION;
    float3 normal : NORMAL;
    float2 uv : UV;
};

float4 main(Input input) : SV_TARGET0
{
    return float4(input.worldPosition.xyz, 1.f);
}
