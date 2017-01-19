struct Output
{
    float4 position : SV_POSITION;
    float2 uv : UV;
};

Output main(uint id : SV_VertexID)
{
    Output output;

    float x = (id == 1 || id == 3);
    float y = (id == 0 || id == 2);
    output.position = float4(x * 2 - 1.f, y * 2 - 1.f, 0.f,  1.f);
    output.uv = float2(x, y);

    return output;
}