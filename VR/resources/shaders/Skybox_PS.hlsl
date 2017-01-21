struct Input
{
    float4 position : SV_POSITION;
    float2 uv : UV;
};

struct Meta
{
    float4x4 projectionMatrix;
    float4x4 viewMatrix;
};

TextureCube txSkybox : register(t0);
StructuredBuffer<Meta> g_Meta : register(t1);

SamplerState samp : register(s0);

float4 main(Input input) : SV_TARGET0
{
    //float3 ndc = inp
    Meta meta = g_Meta[0];
    float4 uvw = float4(input.uv, 1.f, 1.f);
    //float2 uv = input.uv;
    //uv.y = 1.f - uv.y;
    //uv = uv * 2.f - 1.f;
    //float4 uvw = float4(input.uv, 1.f, 1.f);

    uvw = mul(uvw, meta.projectionMatrix);
    //uvw.xy = uvw.xy = 
    uvw = mul(uvw, meta.viewMatrix);
    uvw = normalize(uvw);
    float3 skybox = txSkybox.Sample(samp, uvw.xyz).rgb;
    return float4(skybox, 1.f);
}
